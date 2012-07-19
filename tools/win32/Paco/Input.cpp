//
// Input.cpp
// Paco's input comes from here
//

#include <stdio.h>
#include <stdlib.h>
#include "Input.h"
#include "lif.h"

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

//
// fileSave
// saves data into filename, returns success
//
bool fileSave(char* filename, ubyte* data, sdword length)
{
    FILE* outFile;
    
    outFile = fopen(filename, "wb");
    if (outFile == NULL)
    {
        return false;
    }

    fwrite(data, 1, length, outFile);

    fclose(outFile);

    return true;
}

//
// inLIFFileLoad
// loads and returns a LiF (struct lifheader) given filename
//
lifheader* inLIFFileLoad(char* filename)
{
    ubyte* newData;
	lifheader* newHeader;

	//allocate memory
    newData = new ubyte[fileSizeGet(filename)];
    newHeader = (lifheader*)newData;

	//load the file
	if (!fileLoad(filename, newData))
	{
		return NULL;
	}
	
	//fixup pointers
	newHeader->data = (ubyte*)((udword)newHeader->data + (udword)newHeader);
    if (newHeader->flags & TRF_Paletted)
    {
        if (newHeader->palette != NULL)
        {
	        newHeader->palette = (color*)((udword)newHeader->palette + (udword)newHeader);
        }
        if (newHeader->teamEffect0 != NULL)
        {
            newHeader->teamEffect0 = (ubyte*)((udword)newHeader->teamEffect0 + (udword)newHeader);
        }
        if (newHeader->teamEffect1 != NULL)
        {
            newHeader->teamEffect1 = (ubyte*)((udword)newHeader->teamEffect1 + (udword)newHeader);
        }
    }
    else
    {
        newHeader->palette = NULL;
        if (newHeader->flags & TRF_TeamColor0)
        {
            newHeader->teamEffect0 = (ubyte*)((udword)newHeader->teamEffect0 + (udword)newHeader);
        }
        else
        {
            newHeader->teamEffect0 = NULL;
        }
        if (newHeader->flags & TRF_TeamColor1)
        {
            newHeader->teamEffect1 = (ubyte*)((udword)newHeader->teamEffect1 + (udword)newHeader);
        }
        else
        {
            newHeader->teamEffect1 = NULL;
        }
    }

	//return the LiF
	return newHeader;
}

//
// inLIFFileClose
// frees memory associated with a LiF
//
void inLIFFileClose(lifheader* header)
{
	delete header;
}