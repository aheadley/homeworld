/*
** HFF.CPP : Code to save Homeworld font files from Fontastic.
*/

#include "stdafx.h"

#include "hff.h"
#include "Fontastic.h"
#include "assert.h"
#include "math.h"

SaveHomeworldFontFile::SaveHomeworldFontFile(void)
{
	memset(&fileHeader, 0x00, sizeof(HFFHeader));

	for( unsigned long i=0 ; i<256 ; i++)
	{
		charHeaders[i] = 0;
	}

	fontBitmap = 0;
}

SaveHomeworldFontFile::~SaveHomeworldFontFile(void)
{
	for( unsigned long i=0 ; i<256 ; i++)
	{
		if(charHeaders[i])
		{
			// Should always be one.
			assert(charHeaders[i]->charBuffer);
			
			delete [] charHeaders[i]->charBuffer;

			delete charHeaders[i];
		}
	}

	if(fontBitmap)
		delete [] fontBitmap;
}

signed char SaveHomeworldFontFile::SaveHFF(CString &fileName)
{	
	CreateCharacters();

	CreateCharacterBitmap();
	
	SetupFileHeader();

	SetupFileOffsets();

	WriteFile(fileName);

	return(TRUE);
}

#define HACK_ANGLE				(13)
void SaveHomeworldFontFile::CalculateCharacterExtents(CDC &dc, CFont &fTemp, CString &strTemp, CSize &sTemp)
{
	double rad, shDist;
	LOGFONT lfTemp;
	
	sTemp = dc.GetTextExtent(strTemp);

	fTemp.GetLogFont(&lfTemp);

	if(!lfTemp.lfItalic)
		return;
	
	rad = ((HACK_ANGLE) / 180.0) * 3.14159;

	shDist = sTemp.cy * tan(rad);

	sTemp.cx += (int)shDist;
}

int SaveHomeworldFontFile::CalculateFontSpacing(void)
{
	double rad, shDist;

	if(!gFont.lfItalic)
		return(1);

	rad = ((HACK_ANGLE) / 180.0) * 3.14159;

	shDist = fileHeader.imageHeight * tan(rad);

	return(-((int)shDist - 1));
}

InternalCharacterHeader *SaveHomeworldFontFile::CreateIndividualCharacter(unsigned char c)
{
	CDC dcTemp;
	CString strTemp;
	CBitmap bitmapTemp;
	CFont Font;
	CSize FontSize;
	InternalCharacterHeader	*FinalCharacter, *OriginalCharacter;
	signed long i, j, bufIndex = 0, charTop = -1;
		
	// Create a string for this character.
	strTemp = c;
	
	// Allocate space for the ORIG and small characters.
	FinalCharacter = new InternalCharacterHeader;
	OriginalCharacter = new InternalCharacterHeader;
	
	// Create a device context.
	dcTemp.CreateCompatibleDC(NULL);
	bitmapTemp.CreateCompatibleBitmap(&dcTemp, 400, 400);
	dcTemp.SelectObject(bitmapTemp);
	dcTemp.FillSolidRect(0, 0, 400, 400, RGB(255, 255, 255));
	
	// Create the font
	Font.CreateFontIndirect(&gFont);

	// Draw the font into the device context.
	dcTemp.SelectObject(Font);
	dcTemp.SetTextColor(RGB(0, 0, 0));
	dcTemp.TextOut(0, 0, strTemp);

	// Get the information on this character.
	CalculateCharacterExtents(dcTemp, Font, strTemp, FontSize);

//	FontSize.cx++;
//	FontSize.cy++;

	// Allocate space to hold the final character.(after anitaliasing and dropshadow)
	FinalCharacter->charBufferSize = (FontSize.cx) * (FontSize.cy) * sizeof(unsigned char);
	FinalCharacter->charBuffer = new char [FinalCharacter->charBufferSize];

	// Allocate space to hold the original character.
	OriginalCharacter->charBufferSize = (FontSize.cx) * (FontSize.cy) * sizeof(unsigned char);
	OriginalCharacter->charBuffer = new char [OriginalCharacter->charBufferSize];

	// Copy over the bitmap to buffer and convert it to monochrome.
	for( i = 0; i < FontSize.cy; i++ )
	{
		for( j = 0; j < FontSize.cx; j++, bufIndex ++ )
		{
			COLORREF storeVal = dcTemp.GetPixel(j, i);

			if((GetRValue(storeVal) != 255) || 
				(GetGValue(storeVal) != 255) || 
				(GetBValue(storeVal) != 255))
			{
				OriginalCharacter->charBuffer[bufIndex] = 15;
				
				if(charTop == -1)
					charTop = i;
			}
			else
			{
				OriginalCharacter->charBuffer[bufIndex] = 0;
			}
		}
	}

//	if(enableDropshadow)
//	{
//		FontSize.cx += 3;
//		FontSize.cy += 3;
//	}

	if(enableAntialiasing)
		CharacterAntialias(FinalCharacter->charBuffer, OriginalCharacter->charBuffer, &FontSize);
	else
		CharacterCopy(FinalCharacter->charBuffer, OriginalCharacter->charBuffer, &FontSize);

	if(enableDropshadow)
		AddDropshadow(FinalCharacter->charBuffer, &FontSize);

	// Set up the header for this character.
//	FinalCharacter->charHeader.u		= 0; // Set this up in CreateCharacterBitmap.
	FinalCharacter->charHeader.v		= 0;//(unsigned short)charTop;
	FinalCharacter->charHeader.width	= (unsigned short)FontSize.cx;
	FinalCharacter->charHeader.height	= (unsigned short)FontSize.cy;
	FinalCharacter->charHeader.xOffset	= 0; // Set this up later.
	FinalCharacter->charHeader.yOffset	= 0; // Set this up later.

	// delete original character buffer
	delete [] OriginalCharacter->charBuffer;
	delete OriginalCharacter;
	
	return(FinalCharacter);
}

#define INDEX(a,b,c)	((long)(((floor(c))*(floor(a)))+(floor(b))))
void SaveHomeworldFontFile::CharacterAntialias(char *dst, char *src, CSize *size)
{
	signed long dx, dy, convx, convy;
	float val, sx, sy;
	signed long ipixels;
//	signed long pixels;

	sx = sy = 0;

	// Loop through the ORIG image.
	for( dy = 0; dy < size->cy; dy ++ )
	{
		for( dx = 0; dx < size->cx; dx ++ )
		{
			// Reset val.
			val = 0.0F;

			if(gAntialiasingLevel == 1)		// light antialiasing
			{
				if(src[INDEX(size->cx, sx, sy)] == 0)
				{
					ipixels = 0;
//					pixels = 0;
					for( convy = -1 ; convy < 2; convy++ )
					{
						// Skip this out of bounds row?
						if(sy + convy < 0)
							continue;
						if(sy + convy > size->cy - 1)
							continue;

						for (convx = -1; convx < 2; convx++ )
						{
							// Skip this out of bounds col?
							if(sx + convx < 0)
								continue;
							if(sx + convx > size->cx - 1)
								continue;

							if((convx == 0 || convy == 0) && (src[INDEX(size->cx, sx + convx, sy + convy)]))
								ipixels++;
//							if(src[INDEX(size->cx, sx + convx, sy + convy)])
//								pixels++;
						}
					}
					if(ipixels >= 2)
						val = 5;
						
					dst[INDEX(size->cx, dx, dy)] = (char)val;
				}
				else
				{
					ipixels = 0;
//					pixels = 0;
					for( convy = -1 ; convy < 2; convy++ )
					{
						// Skip this out of bounds row?
						if(sy + convy < 0)
							continue;
						if(sy + convy > size->cy - 1)
							continue;

						for (convx = -1; convx < 2; convx++ )
						{
							// Skip this out of bounds col?
							if(sx + convx < 0)
								continue;
							if(sx + convx > size->cx - 1)
								continue;

							if((convx == 0 || convy == 0) && (convx != 0 || convy != 0) && (src[INDEX(size->cx, sx + convx, sy + convy)]))
								ipixels++;
//							if(src[INDEX(size->cx, sx + convx, sy + convy)])
//								pixels++;
						}
					}
					if(ipixels == 2 && sx > 0 && sy > 0 && src[INDEX(size->cx, sx-1, sy)] && src[INDEX(size->cx, sx, sy-1)])
						val = 11;
					else
					if(ipixels == 2 && sx < (size->cx - 1) && sy > 0 && src[INDEX(size->cx, sx+1, sy)] && src[INDEX(size->cx, sx, sy-1)])
						val = 11;
					else
					if(ipixels == 2 && sx < (size->cx - 1) && sy < (size->cy - 1) && src[INDEX(size->cx, sx+1, sy)] && src[INDEX(size->cx, sx, sy+1)])
						val = 11;
					else
					if(ipixels == 2 && sx > 0 && sy < (size->cy - 1) && src[INDEX(size->cx, sx-1, sy)] && src[INDEX(size->cx, sx, sy+1)])
						val = 11;
					else
						val = 15;
						
					dst[INDEX(size->cx, dx, dy)] = (char)val;
				}
			}
			else	// regular (strong) antialiasing
			{
				for( convy = -1 ; convy < 2; convy++ )
				{
					// Skip this out of bounds row?
					if(sy + convy < 0)
						continue;
					if(sy + convy > size->cy - 1)
						continue;

					for (convx = -1; convx < 2; convx++ )
					{
						// Skip this out of bounds col?
						if(sx + convx < 0)
							continue;
						if(sx + convx > size->cx - 1)
							continue;

						if(convx == 0 && convy == 0)
							val += (float)src[INDEX(size->cx, sx + convx, sy + convy)];
						else
						if(convx == 0 || convy == 0)
							val += ((float)src[INDEX(size->cx, sx + convx, sy + convy)]) / 3.0F;
						else
							val += ((float)src[INDEX(size->cx, sx + convx, sy + convy)]) / 4.0F;
					}
				}
				val = (val / 1.85F);
//				if(src[INDEX(size->cx, sx, sy)] > 0)
//					val += 2;
				if(val > 15)
					val = 15;
				dst[INDEX(size->cx, dx, dy)] = (char)val;
			}
			assert(INDEX(size->cx, dx, dy) < size->cx * size->cy);
			sx++;
		}
		sy++;
		sx = 0;
	}
}
void SaveHomeworldFontFile::CharacterCopy(char *dst, char *src, CSize *size)
{
	signed long dx, dy;
	float sx = 0, sy = 0;

	// Loop through the ORIG image and copy to the final buffer.
	for( dy = 0; dy < size->cy; dy++ )
	{
		for( dx = 0; dx < size->cx; dx++ )
		{
			dst[INDEX(size->cx, dx, dy)] = src[INDEX(size->cx, sx, sy)];
			assert(INDEX(size->cx, dx, dy) < size->cx * size->cy);
			sx++;
		}
		sy++;
		sx = 0;
	}
}

void SaveHomeworldFontFile::AddDropshadow(char *dst, CSize *size)
{
	signed long dx, dy;
	unsigned char pixelsource;

	// Loop through the image.
	for( dy = 1 ; dy < size->cy ; dy++ )
		for( dx = 1 ; dx < size->cx ; dx++ )
		{
			pixelsource = dst[INDEX(size->cx, dx - 1, dy - 1)];				// 1 pixel away
			if((pixelsource > 2) && (dst[INDEX(size->cx, dx, dy)] == 0))
				dst[INDEX(size->cx, dx, dy)] = 2;
//			if(dx > 1 && dy > 1)
//			{
//				pixelsource = dst[INDEX(size->cx, dx - 2, dy - 2)];			// 2 pixels away
//				if((pixelsource > 2) && (dst[INDEX(size->cx, dx, dy)] == 0))
//					dst[INDEX(size->cx, dx, dy)] = 2;
//			}
		}
}

signed char SaveHomeworldFontFile::CreateCharacters(void)
{
	unsigned long i;
	unsigned char charIndex;

	// Create all the characters.
	for( i=0 ; i<strlen(chooseTextTemplate) ; i++ )
	{
		charIndex = chooseTextTemplate.GetAt(i);

		charHeaders[charIndex] = CreateIndividualCharacter(charIndex);
	}

	return(TRUE);
}

signed char SaveHomeworldFontFile::CreateCharacterBitmap(void)
{
	unsigned long i, j, k, xOff = 0;
	char *pTemp, *pDst, *pSrc;

	fileHeader.imageWidth = fileHeader.imageHeight = 0;
	fileHeader.height = 0;
	fileHeader.yBaseline = 0;

	for( i=0 ; i<256 ; i++ )
	{
		// Is this one allocated?
		if(charHeaders[i])
		{
			if(charHeaders[i]->charHeader.height > fileHeader.height)
				fileHeader.height = charHeaders[i]->charHeader.height;

			fileHeader.imageWidth += charHeaders[i]->charHeader.width;

			if((i >= 65) && (i <= 90))
				fileHeader.yBaseline = charHeaders[i]->charHeader.height;
		}
	}

	fileHeader.imageHeight = fileHeader.height;
	fontBitmap = new char [fileHeader.imageWidth * fileHeader.imageHeight * sizeof(char)];
	memset(fontBitmap, 0x00, fileHeader.imageWidth * fileHeader.imageHeight * sizeof(char));

	pTemp = fontBitmap;

	for( i=0 ; i<256 ; i++ )
	{
		// Is this one allocated?
		if(charHeaders[i])
		{
			pDst = pTemp;
			pSrc = charHeaders[i]->charBuffer;
			
			// Copy it over.
			for( j=0 ; j<charHeaders[i]->charHeader.height ; j++ )
			{
				for( k=0 ; k<charHeaders[i]->charHeader.width ; k++ )
				{
					pDst[k] = pSrc[k];
				}

				pDst += fileHeader.imageWidth;
				pSrc += charHeaders[i]->charHeader.width;
			}

			// Set the UV coords for this character.
			charHeaders[i]->charHeader.u = (unsigned short)xOff;
			
			pTemp += charHeaders[i]->charHeader.width;
			xOff += charHeaders[i]->charHeader.width;
		}
	}

	return(TRUE);
}

signed char SaveHomeworldFontFile::SetupFileHeader(void)
{
	strcpy(fileHeader.identity, HFF_FILE_ID);
	fileHeader.version =		HFF_FILE_VERSION;
	fileHeader.flags =			0; // None for now.
	fileHeader.nCharacters =	strlen(chooseTextTemplate);
	fileHeader.spacing =		CalculateFontSpacing();
//	fileHeader.height =			0; // Set this up below.
//	fileHeader.yBaseline =		0; // Set this up in CreateCharacterBitmap.
	fileHeader.oName =			0; // Set this up later.
//	fileHeader.imageWidth =		0; // Set this up in CreateCharacterBitmap.
//	fileHeader.imageHeight =	0; // Set this up in CreateCharacterBitmap.
	fileHeader.nColors =		2; // Set this up later.
	fileHeader.oPalette =		0; // Set this up later.
	fileHeader.oImage =			0; // Set this up later.
	memset(&fileHeader.reserved, 0xAB, 2);
	memset(&fileHeader.oCharacters, 0x00, sizeof(unsigned long) * 256);

	return(TRUE);
}

signed char SaveHomeworldFontFile::SetupFileOffsets(void)
{
	unsigned long fileOffset = 0, i;

	// skip the file header.
	fileOffset += sizeof(HFFHeader);

	// Set the offsets for the individual characters.
	for( i=0 ; i<256 ; i++ )
	{
		if(charHeaders[i])
		{
			fileHeader.oCharacters[i] = fileOffset;

			fileOffset += sizeof(CharacterHeader);
		}
	}

	fileHeader.oImage = fileOffset;

	fileOffset += fileHeader.imageWidth * fileHeader.imageHeight * sizeof(unsigned char);

	fileHeader.oPalette = fileOffset;

	return(TRUE);
}

signed char SaveHomeworldFontFile::WriteFile(CString &fileName)
{
	unsigned long i;
	CFile fileOutput(fileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);

	// Write out the header.
	fileOutput.Write(&fileHeader, sizeof(HFFHeader));
	
	// Write out the character headers.
	for( i=0 ; i<256 ; i++ )
	{
		if(charHeaders[i])
		{
			fileOutput.Write(&charHeaders[i]->charHeader, sizeof(CharacterHeader));
		}
	}

	// Write out the character image.
	fileOutput.Write(fontBitmap, fileHeader.imageWidth * fileHeader.imageHeight * sizeof(unsigned char));

	// Write out the palette.
	unsigned long c0=0x00000000, c1=0x00ffffff;

	fileOutput.Write(&c0, sizeof(unsigned long));
	fileOutput.Write(&c1, sizeof(unsigned long));

	return(TRUE);
}
