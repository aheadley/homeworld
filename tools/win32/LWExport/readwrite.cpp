/*
** READWRITE.CPP : Code to handle reading and writing atomic types to and from files.
*/

#include "readwrite.h"

unsigned char ReadByte(FILE *fileHandle)
{
	return(fgetc(fileHandle));
}

unsigned short ReadWord(FILE *fileHandle)
{
	unsigned short retVal, b0, b1;

	b0 = ReadByte(fileHandle);
	b1 = ReadByte(fileHandle);

	retVal = (b0 << 8) + b1;

	return(retVal);
}

unsigned long ReadLong(FILE *fileHandle)
{
	unsigned long retVal, b0, b1;

	b0 = ReadWord(fileHandle);
	b1 = ReadWord(fileHandle);

	retVal = (b0 << 16) + b1;

	return(retVal);
}

float ReadFloat(FILE *fileHandle)
{
	unsigned long temp, b0, b1, b2, b3;
	float retVal;

	b0 = fgetc(fileHandle);
	b1 = fgetc(fileHandle);
	b2 = fgetc(fileHandle);
	b3 = fgetc(fileHandle);

	temp = (b0 << 24) + (b1 << 16) + (b2 << 8) + (b3);

	retVal = *((float *)(&temp));

	return(retVal);
}

char *ReadString(FILE *fileHandle)
{
	char *tempStr = new char [255];
	unsigned long index = 0;

	while(tempStr[index++] = ReadByte(fileHandle));

	if(index % 2)
		ReadByte(fileHandle);

	return(tempStr);
}

void ReadTextLine(char *lineBuffer, FILE *fileHandle)
{
	unsigned long iBuffer = 0;

	while(!feof(fileHandle))
	{
		lineBuffer[iBuffer] = ReadByte(fileHandle);

		if(lineBuffer[iBuffer] == 0x0A) // Found a C/R?
		{
			// Terminate the string.
			lineBuffer[iBuffer] = 0x00;

			return;
		}

		iBuffer ++;
	}
}

void ReadIFFTag(char *tagBuffer, FILE *fileHandle)
{
	fread(tagBuffer, 1, 4, fileHandle);

	tagBuffer[4] = 0x00;
}