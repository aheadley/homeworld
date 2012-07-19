//
// Main.cpp
// Paco's top-level stuff
//

#include <stdio.h>
#include <stdlib.h>
#include "Main.h"
#include "Input.h"
#include "list.h"

list_lif liflist;
list_lif nopacklist;
list_lif notpackedlist;
list_lifpage lifpagelist;

//
// mainStartupOnce
// initialize internals once per Paco invocation
//
bool mainStartupOnce(void)
{
    meshLoadLIFList();
    return true;
}

//
// mainStartup
// initialize Paco's internals
//
bool mainStartup(void)
{
	//clear the liflist
	lifpagelist.erase(lifpagelist.begin(), lifpagelist.end());
	liflist.erase(liflist.begin(), liflist.end());
    nopacklist.erase(nopacklist.begin(), nopacklist.end());
    notpackedlist.erase(notpackedlist.begin(), notpackedlist.end());

    return true;
}

//
// mainLoadTextures
// read list of LiFs from a file
//
bool mainLoadTextures(char* filename)
{
	liflist_t element;
	FILE* file;
	char line[512];

	//open the listing file
	file = fopen(filename, "rt");
	if (file == NULL)
	{
		return false;
	}

	//fill the liflist
	while (!feof(file))
	{
		//get next line of input file
		fgets(line, 511, file);
		if (strlen(line) < 16)
		{
			continue;
		}
        if (feof(file))
        {
            break;
        }

        //check for comment
        if (line[0] == ';')
        {
            continue;
        }

		//chomp the cr
        if (line[strlen(line) - 1] == '\n')
        {
		    line[strlen(line) - 1] = '\0';
        }
		
		//fill our structure
		memset(&element, 0, sizeof(liflist_t));
		strncpy(element.filename, line, 1023);
        element.crc = crc32ComputeString(line);
		element.header = inLIFFileLoad(element.filename);
		
		//add to liflist
        if (element.header != NULL)
        {
            liflist.push_back(element);
        }
	}

	//close the listing file
	fclose(file);

	return true;
}

bool mainTextureAlreadyLoaded(char* name)
{
    //too bad STL maps can't be used interchangeably w/ lists
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if (!strcmp(name, (*i).filename))
        {
            return true;
        }
    }
    return false;
}

//
// mainGetTextureCRC
// return the palette CRC of a given LiF
//
crc32 mainGetTextureCRC(char* filename)
{
    liflist_t element;

    memset(&element, 0, sizeof(liflist_t));
    strncpy(element.filename, filename, 1023);
    element.header = inLIFFileLoad(element.filename);

    if (element.header == NULL)
    {
        fprintf(stderr, "\n-- CRC couldn't load file %s --\n", filename);
        return false;
    }

    if (element.header->flags & TRF_Paletted)
    {
        return element.header->paletteCRC;
    }
    else
    {
        return 0;
    }
}

//
// mainLoadTexture
// load a single LiF
//
bool mainLoadTexture(char* filename, bool alpha)
{
    liflist_t element;

    //fill our structure
    memset(&element, 0, sizeof(liflist_t));
    strncpy(element.filename, filename, 1023);
    element.crc = crc32ComputeString(filename);
    element.header = inLIFFileLoad(element.filename);

    if (element.header == NULL)
    {
        fprintf(stderr, "\n-- TEX couldn't load file %s --\n", filename);
        return false;
    }

    if (mainTextureAlreadyLoaded(element.filename))
    {
        return true;
    }

    if (alpha)
    {
        if (element.header->flags & TRF_Paletted)
        {
            return true;
        }
    }
    else
    {
        if (!(element.header->flags & TRF_Paletted))
        {
            return true;
        }
    }

    //add to liflist
    if (element.header != NULL)
    {
        liflist.push_back(element);
        return true;
    }
    else
    {
        return false;
    }
}

bool mainLoadTextureWithCRC(char* filename, crc32 crc)
{
    liflist_t element;

    memset(&element, 0, sizeof(liflist_t));
    strncpy(element.filename, filename, 1023);
    element.crc = crc32ComputeString(filename);
    element.header = inLIFFileLoad(element.filename);

    if (element.header == NULL)
    {
        fprintf(stderr, "\n-- w/CRC couldn't load file %s --\n", filename);
        return false;
    }

    if (mainTextureAlreadyLoaded(element.filename))
    {
        return true;
    }

    if (element.header->flags & TRF_Paletted)
    {
        if (element.header->paletteCRC == crc)
        {
            liflist.push_back(element);
            return true;
        }
    }

    return false;
}

//
// mainClassifyTextures
// classify list of LiFs
//
bool mainClassifyTextures(void)
{
    lifheader* lif;

    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        lif = (*i).header;
        if (lif != NULL)
        {
            if (lif->width == lif->height)
            {
                (*i).flags |= PACF_Square;
            }
            if (lif->width > 2*lif->height ||
                lif->height > 2*lif->width)
            {
                (*i).flags |= PACF_Strange;
            }
        }
    }

    return true;
}

//
// mainShutdown
// free up Paco's structures
//
bool mainShutdown(void)
{
	//free memory in liflist
	for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
	{
		if ((*i).header != NULL)
		{
			inLIFFileClose((*i).header);
		}
	}

	//clear the liflist
	lifpagelist.erase(lifpagelist.begin(), lifpagelist.end());
	liflist.erase(liflist.begin(), liflist.end());

	return true;
}
