//
// file.cpp
//

#include <stdio.h>
#include "file.h"

//
// fileSizeGet
// returns the length of the given filename
//
sdword fileSizeGet(char* filename)
{
	FILE* file;
	sdword length;

	file = fopen(filename, "rb");
	if (file == NULL)
	{
		return 0;
	}

	length = fseek(file, 0, SEEK_END);
	length = ftell(file);
	fclose(file);

	return length;
}

//
// fileLoad
// loads filename into data, returns success
//
bool fileLoad(char* filename, ubyte* data)
{
	FILE* inFile;
	sdword length;

	length = fileSizeGet(filename);

	inFile = fopen(filename, "rb");
	if (inFile == NULL)
	{
		return false;
	}

	fread(data, 1, length, inFile);

	fclose(inFile);

	return true;
}
