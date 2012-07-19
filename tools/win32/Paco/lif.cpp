//
// lif.cpp
// Paco's LiF routines
//

#include "lif.h"
#include "list.h"
#include "crc.h"
#include "Paco.h"
#include "Input.h"
#include "tga.h"

#define HAS_TEAM_EFFECT0(LIF) ((LIF->teamEffect0 != NULL) || (LIF->flags & TRF_TeamColor0))
#define HAS_TEAM_EFFECT1(LIF) ((LIF->teamEffect1 != NULL) || (LIF->flags & TRF_TeamColor1))

#define PIXEL(BUF,X,Y,STRIDE) BUF[(Y)*(STRIDE) + (X)]

//
// return the first LiF on a page
//
lifheader* lifFirstLIFOnPage(lifpage* page)
{
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent == page)
        {
            //found one
            return (*i).header;
        }
    }
    
    //no LiFs on the page
    return NULL;
}

lifheader* lifFirstTeam0OnPage(lifpage* page)
{
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent == page)
        {
            if (HAS_TEAM_EFFECT0((*i).header))
            {
                return (*i).header;
            }
        }
    }

    return NULL;
}

lifheader* lifFirstTeam1OnPage(lifpage* page)
{
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent == page)
        {
            if (HAS_TEAM_EFFECT1((*i).header))
            {
                return (*i).header;
            }
        }
    }

    return NULL;
}

void lifGatherFlags(lifpage* page, sdword& flags)
{
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent == page)
        {
            if (HAS_TEAM_EFFECT0((*i).header))
            {
                flags |= TRF_TeamColor0;
            }
            if (HAS_TEAM_EFFECT1((*i).header))
            {
                flags |= TRF_TeamColor1;
            }
        }
    }
}

//
// lifBlit
// blit one LiF into another
//
void lifBlit(lifheader* out, lifheader* in, sdword left, sdword top)
{
    sdword y, x;

    for (y = 0; y < in->height; y++)
    {
        for (x = 0; x < in->width; x++)
        {
            PIXEL(out->data, left + x, top + y, out->width) = PIXEL(in->data, x, y, in->width);
        }
    }
}

//
// lifLongBlit
// blit one RGBA LiF into another
//
void lifLongBlit(lifheader* out, lifheader* in, sdword left, sdword top)
{
    sdword y, x;
    color* sp;
    color* dp;

    sp = (color*)in->data;
    dp = (color*)out->data;

    for (y = 0; y < in->height; y++)
    {
        for (x = 0; x < in->width; x++)
        {
            PIXEL(dp, left + x, top + y, out->width) = PIXEL(sp, x, y, in->width);
        }
    }
}

void lifBlit0(lifheader* out, lifheader* in, sdword left, sdword top)
{
    sdword y, x;

    if (in->teamEffect0 == NULL)
    {
        return;
    }

    for (y = 0; y < in->height; y++)
    {
        for (x = 0; x < in->width; x++)
        {
            PIXEL(out->teamEffect0, left + x, top + y, out->width) = PIXEL(in->teamEffect0, x, y, in->width);
        }
    }
}

void lifBlit1(lifheader* out, lifheader* in, sdword left, sdword top)
{
    sdword y, x;

    if (in->teamEffect1 == NULL)
    {
        return;
    }

    for (y = 0; y < in->height; y++)
    {
        for (x = 0; x < in->width; x++)
        {
            PIXEL(out->teamEffect1, left + x, top + y, out->width) = PIXEL(in->teamEffect1, x, y, in->width);
        }
    }
}

void lifExportAlphaPage(lifpage* page, char* filePrefix)
{
    lifheader* newLif;
    lifheader* lif;
    ubyte* teamEffect0;
    ubyte* teamEffect1;
    ubyte* data;
    sdword size;

    newLif = new lifheader;
    lif = lifFirstLIFOnPage(page);
    memcpy(newLif, lif, sizeof(lifheader));

    teamEffect0 = new ubyte[page->width * page->height];
    teamEffect1 = new ubyte[page->width * page->height];
    memset(teamEffect0, 0, page->width * page->height);
    memset(teamEffect1, 0, page->width * page->height);

    data = new ubyte[sizeof(color) * page->width * page->height];
    memset(data, 0, sizeof(color) * page->width * page->height);

    newLif->flags = TRF_Alpha;
    lifGatherFlags(page, newLif->flags);

    newLif->width = page->width;
    newLif->height = page->height;

    newLif->data = data;
    newLif->palette = NULL;
    newLif->teamEffect0 = teamEffect0;
    newLif->teamEffect1 = teamEffect1;

    //
    //.. copy LiF data into the larger bitmap ..
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent != page)
        {
            continue;
        }

        //blit data
        lifLongBlit(newLif, (*i).header, (*i).packedX, (*i).packedY);
        //blit team effect 0
        if (HAS_TEAM_EFFECT0((*i).header))
        {
            lifBlit0(newLif, (*i).header, (*i).packedX, (*i).packedY);
        }
        //blit team effect 1
        if (HAS_TEAM_EFFECT1((*i).header))
        {
            lifBlit1(newLif, (*i).header, (*i).packedX, (*i).packedY);
        }
    }

    page->header = newLif;

    //
    //generate CRCs for the large bitmap
    newLif->imageCRC = crc32Compute(data, sizeof(color) * page->width * page->height);

    if (!exportPages)
    {
        goto DONT_EXPORT;
    }

    //
    //save the page
    FILE* out;
    char fullName[1024];
    strcpy(fullName, filePrefix);
    strcat(fullName, page->name);
    strcat(fullName, ".lif");
    out = fopen(fullName, "wb");
    if (out == NULL)
    {
        if (!quietMode)
        {
            printf("couldn't create %s.lif\n", page->name);
        }
    }
    else
    {
        size = sizeof(lifheader);
        newLif->data = (ubyte*)size;
        size += sizeof(color) * page->width * page->height;

        newLif->palette = NULL;

        if (newLif->flags & TRF_TeamColor0)
        {
            newLif->teamEffect0 = (ubyte*)size;
            size += page->width * page->height;
        }
        else
        {
            newLif->teamEffect0 = NULL;
        }
        
        if (newLif->flags & TRF_TeamColor1)
        {
            newLif->teamEffect1 = (ubyte*)size;
        }
        else
        {
            newLif->teamEffect1 = NULL;
        }
        
        //header
        fwrite(newLif, sizeof(lifheader), 1, out);

        //data
        fwrite(data, sizeof(color) * page->width * page->height, 1, out);

        //team effect 0
        if (newLif->flags & TRF_TeamColor0)
        {
            fwrite(teamEffect0, page->width * page->height, 1, out);
        }

        //team effect 1
        if (newLif->flags & TRF_TeamColor1)
        {
            fwrite(teamEffect1, page->width * page->height, 1, out);
        }

        fclose(out);
    }

DONT_EXPORT:
    
    if (pacDumpTargas)
    {
        newLif->data = data;
        strcpy(fullName, filePrefix);
        strcat(fullName, page->name);
        strcat(fullName, ".tga");
        tgaExportLIF(newLif, fullName);
    }

    //
    //free memory
    delete [] teamEffect0;
    delete [] teamEffect1;
    delete [] data;
    delete newLif;
}

void lifExportPalettedPage(lifpage* page, char* filePrefix)
{
    lifheader* newLif;
    lifheader* lif;
    color* palette;
    ubyte* teamEffect0;
    ubyte* teamEffect1;
    ubyte* data;
    sdword size;

    //
    //create a new LiF header w/ teamEffect[01] copied
    //from any LiF on the page, same w/ palette.
    //copy the palette CRC

    newLif = new lifheader;
    lif = lifFirstLIFOnPage(page);
    palette = new color[256];
    teamEffect0 = new ubyte[256];
    teamEffect1 = new ubyte[256];
    memset(palette, 0, 256 * sizeof(color));
    memset(teamEffect0, 0, 256);
    memset(teamEffect1, 0, 256);

    data = new ubyte[page->width * page->height];
    memset(data, 0, page->width * page->height);
    
    memcpy(newLif, lif, sizeof(lifheader));

    newLif->flags = TRF_Paletted;
    lifGatherFlags(page, newLif->flags);

    newLif->width = page->width;
    newLif->height = page->height;
    newLif->paletteCRC = lif->paletteCRC;

    //palettes & team effects are the same for all images on the page
    memcpy(palette, lif->palette, 256 * sizeof(color));
    if (newLif->flags & TRF_TeamColor0)
    {
        lifheader* lif0 = lifFirstTeam0OnPage(page);
        memcpy(teamEffect0, lif0->teamEffect0, 256);
    }
    if (newLif->flags & TRF_TeamColor1)
    {
        lifheader* lif1 = lifFirstTeam1OnPage(page);
        memcpy(teamEffect1, lif1->teamEffect1, 256);
    }

    newLif->palette = palette;
    newLif->data = data;

    //
    //.. copy LiF data into the larger bitmap ..
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent != page)
        {
            //not ours
            continue;
        }

        lifBlit(newLif, (*i).header, (*i).packedX, (*i).packedY);
    }

    page->header = newLif;

    //
    //generate CRCs for the large bitmap
    newLif->imageCRC = crc32Compute(data, page->width * page->height);

    if (!exportPages)
    {
        goto DONT_EXPORT;
    }

    //
    //save the page
    FILE* out;
    char fullName[1024];
    strcpy(fullName, filePrefix);
    strcat(fullName, page->name);
    strcat(fullName, ".lif");
    out = fopen(fullName, "wb");
    if (out == NULL)
    {
        if (!quietMode)
        {
            printf("couldn't create %s.lif\n", page->name);
        }
    }
    else
    {
        size = sizeof(lifheader);

        newLif->data = (ubyte*)size;
        size += page->width * page->height;

        newLif->palette = (color*)size;
        size += 256 * sizeof(color);

        if (newLif->flags & TRF_TeamColor0)
        {
            newLif->teamEffect0 = (ubyte*)size;
            size += 256;
        }
        else
        {
            newLif->teamEffect0 = NULL;
        }

        if (newLif->flags & TRF_TeamColor1)
        {
            newLif->teamEffect1 = (ubyte*)size;
        }
        else
        {
            newLif->teamEffect1 = NULL;
        }
        
        //header
        fwrite(newLif, sizeof(lifheader), 1, out);

        //data
        fwrite(data, page->width * page->height, 1, out);

        //palette
        fwrite(palette, 256 * sizeof(color), 1, out);

        //team effect 0
        if (newLif->flags & TRF_TeamColor0)
        {
            fwrite(teamEffect0, 256, 1, out);
        }

        //team effect 1
        if (newLif->flags & TRF_TeamColor1)
        {
            fwrite(teamEffect1, 256, 1, out);
        }
        
        fclose(out);
    }

DONT_EXPORT:

    if (pacDumpTargas)
    {
        newLif->data = data;
        newLif->palette = palette;
        strcpy(fullName, filePrefix);
        strcat(fullName, page->name);
        strcat(fullName, ".tga");
        tgaExportLIF(newLif, fullName);
    }

    //
    //free memory
    delete [] palette;
    delete [] teamEffect0;
    delete [] teamEffect1;
    delete [] data;
    delete newLif;
}

//
// lifExportPages
// build pages as actual LiFs & output them
//
void lifExportPages(char* filePrefix)
{
    lifpage* page;

    for (list_lifpage::iterator pit = lifpagelist.begin(); pit != lifpagelist.end(); ++pit)
    {
        page = *pit;

        if (exportPages)
        {
            if (page->flags & PACF_Alpha)
            {
                lifExportAlphaPage(page, filePrefix);
            }
            else
            {
                lifExportPalettedPage(page, filePrefix);
            }
        }
    }
}

//
// lifSbortName
//
char _lifShortName[128];
char* lifShortName(char const* fullname)
{
    char  _lifFullName[512];
    char* token;
    char* lastToken;

    _lifShortName[0] = '\0';
    strncpy(_lifFullName, fullname, 512 - 1);
    token = strtok(_lifFullName, " \\/");
    lastToken = NULL;
    while (token != NULL)
    {
        lastToken = token;
        token = strtok(NULL, " \\/");
    }

    if (lastToken != NULL)
    {
        strncpy(_lifShortName, lastToken, 128 - 1);
    }
    for (sdword i = 0; i < strlen(_lifShortName); i++)
    {
        if (_lifShortName[i] == '.')
        {
            _lifShortName[i] = '\0';
            break;
        }
    }
    return _lifShortName;
}
