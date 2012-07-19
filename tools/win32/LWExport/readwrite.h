/*
** READWRITE.H : Header file for READWRITE.CPP.
*/

#ifndef __READWRITE_H
#define __READWRITE_H

#include "stdio.h"

	unsigned char ReadByte(FILE *fileHandle);
	unsigned short ReadWord(FILE *fileHandle);
	unsigned long ReadLong(FILE *fileHandle);
	float ReadFloat(FILE *fileHandle);

	char *ReadString(FILE *fileHandle);

	void ReadTextLine(char *lineBuffer, FILE *fileHandle);

	void ReadIFFTag(char *tagBuffer, FILE *fileHandle);

#endif