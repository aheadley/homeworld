/*
** SCALY.CPP : Code to scale .LWO files.
*/

#include "stdio.h"
#include "stdlib.h"
#include "process.h"

#include "scaly.h"

float ReadFloat(char *fileBuffer)
{
	unsigned long temp, b0, b1, b2, b3;
	float retVal;

	b0 = (unsigned char)fileBuffer[0];
	b1 = (unsigned char)fileBuffer[1];
	b2 = (unsigned char)fileBuffer[2];
	b3 = (unsigned char)fileBuffer[3];

	temp = (b0 << 24) + (b1 << 16) + (b2 << 8) + (b3);

	retVal = *((float *)(&temp));

	return(retVal);
}

void WriteFloat(char *fileBuffer, float *f)
{
	char *tempBuffer;

	tempBuffer = (char *)f;

	fileBuffer[0] = tempBuffer[3];
	fileBuffer[1] = tempBuffer[2];
	fileBuffer[2] = tempBuffer[1];
	fileBuffer[3] = tempBuffer[0];
}

void main(int argc, char *argv[])
{
	FILE *fileHandle;
	char *fileBuffer, *pTemp;
	char *fileName;
	unsigned long fileLength, i;
	float tsizX, tsizY, tsizZ, scaleFactor;
	
	// Check for a filename argument.
	if(argc < 3)
	{
		printf("must supply a filename and a scalefactor, fool!\n");
		exit(1);
	}

	// Save a pointer to the filename.
	fileName = argv[1];

	// Get the scalefactor.
	scaleFactor = (float)atof(argv[2]) / 100;

	// Open the file
	fileHandle = fopen(fileName, "rb");

	if(!fileHandle)
	{
		printf("unable to open specified file, fool!\n");
		exit(1);
	}

	// Get the filelength
	fseek(fileHandle, 0L, SEEK_END);
	fileLength = ftell(fileHandle);
	fseek(fileHandle, 0L, SEEK_SET);

	// Allocate space for the file.
	fileBuffer = new char [fileLength];

	// Read it in.
	fread(fileBuffer, 1, fileLength, fileHandle);

	// Close the file.
	fclose(fileHandle);

	for( i=0 ; i<fileLength-4 ; i++ )
	{
		// Is this the tsiz chunk?
		if( (fileBuffer[i+0] == 'T') && 
			(fileBuffer[i+1] == 'S') && 
			(fileBuffer[i+2] == 'I') && 
			(fileBuffer[i+3] == 'Z'))
		{
			pTemp = fileBuffer + i + 4 + 2;
			// The next 12 bytes are the scaling for the texture map values.
			tsizX = ReadFloat(pTemp);
			tsizY = ReadFloat(pTemp + 4);
			tsizZ = ReadFloat(pTemp + 8);

			// Scale the textures.
			tsizX *= scaleFactor;
			tsizY *= scaleFactor;
			tsizZ *= scaleFactor;

			WriteFloat(pTemp, &tsizX);
			WriteFloat(pTemp + 4, &tsizY);
			WriteFloat(pTemp + 8, &tsizZ);
		}

		// Is this the tctr chunk?
		if( (fileBuffer[i+0] == 'T') && 
			(fileBuffer[i+1] == 'C') && 
			(fileBuffer[i+2] == 'T') && 
			(fileBuffer[i+3] == 'R'))
		{
			pTemp = fileBuffer + i + 4 + 2;
			// The next 12 bytes are the scaling for the texture map values.
			tsizX = ReadFloat(pTemp);
			tsizY = ReadFloat(pTemp + 4);
			tsizZ = ReadFloat(pTemp + 8);

			// Scale the textures.
			tsizX *= scaleFactor;
			tsizY *= scaleFactor;
			tsizZ *= scaleFactor;

			WriteFloat(pTemp, &tsizX);
			WriteFloat(pTemp + 4, &tsizY);
			WriteFloat(pTemp + 8, &tsizZ);
		}
	}

	// Done scaling.  Re-write the file.
	fileHandle = fopen(fileName, "wb");

	if(!fileHandle)
	{
		printf("unable to open specified file, fool!\n");
		exit(1);
	}

	fwrite(fileBuffer, 1, fileLength, fileHandle);

	fclose(fileHandle);

	delete [] fileBuffer;
}