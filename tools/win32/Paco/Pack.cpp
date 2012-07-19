//
// Pack.cpp
// Paco's packing routines
//

#include <assert.h>
#include "list.h"
#include "paco.h"

#define MAX(A,B) (((A) > (B)) ? (A) : (B))
#define MIN(A,B) (((A) < (B)) ? (A) : (B))

#define PIXEL(D, X, Y, W) D[Y*W + X]

#define pacArea(LIF) (LIF->header->width * LIF->header->height)

#define LARGE_MANHATTAN 192
#define SMALL_MANHATTAN 64

#define THRESH_LARGE 0.85f
#define THRESH_SMALL 0.52f

static sdword stripMaxWidth;
static sdword stripMaxHeight;

#define PLACE_LIF(LIF, PAGE, X, Y) \
    { \
        LIF->flags |= PACF_Packed; \
        LIF->packedX = X; \
        LIF->packedY = Y; \
        LIF->parent = PAGE; \
    }

bool pacStrip(lifpage* page, sdword& stripWidth, bool StripSelection, bool constrained = false, sdword stripMaxWidth = 0, sdword stripMaxHeight = 0);

void pacVerticalSubStrip(lifpage* page, sdword startX, sdword startY, sdword& stripWidth, sdword& stripHeight, sdword maxWidth, sdword maxHeight);
void pacHorizontalSubStrip(lifpage* page, sdword startX, sdword startY, sdword& stripWidth, sdword& stripHeight, sdword maxWidth, sdword maxHeight);

sdword pacNextPowerOf2(sdword n)
{
    if (n <= 1) return 1;
    if (n <= 2) return 2;
    if (n <= 4) return 4;
    if (n <= 8) return 8;
    if (n <= 16) return 16;
    if (n <= 32) return 32;
    if (n <= 64) return 64;
    if (n <= 128) return 128;
    return 256;
}

sdword pacHeightOfPage(lifpage* page)
{
    sdword height, maxHeight;

    maxHeight = 0;

    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent == page)
        {
            height = (*i).header->height + (*i).packedY;
            if (height > maxHeight)
            {
                maxHeight = height;
            }
        }
    }

    return maxHeight;
}

bool pacTexturesRemaining(void)
{
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if (!((*i).flags & PACF_Packed))
        {
            return true;
        }
    }

    return false;
}

//
// pacSelectFirst
// select the 1st texture to place in a strip,
// the one with the largest area of textures with
// the largest dimension
//
liflist_t* pacSelectFirst(sdword dim0, sdword dim1)
{
    liflist_t* largest;
    liflist_t* current;
    sdword currentMaxDim, largestMaxDim;
    sdword constraintMax, constraintMin;
    sdword dimMax, dimMin;

    constraintMax = MAX(dim0, dim1);
    constraintMin = MIN(dim0, dim1);

    largest = NULL;

    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        current = &(*i);

        if (current->flags & PACF_Packed)
        {
            continue;
        }

        //constraint test
        dimMax = MAX(current->header->width, current->header->height);
        dimMin = MIN(current->header->width, current->header->height);
        if (dimMax > constraintMax ||
            dimMin > constraintMin)
        {
            continue;
        }

        if (largest == NULL)
        {
            largest = current;
        }
        else
        {
            currentMaxDim = MAX(current->header->width, current->header->height);
            largestMaxDim = MAX(largest->header->width, largest->header->height);
            if (currentMaxDim > largestMaxDim)
            {
                largest = current;
            }
            else if (currentMaxDim == largestMaxDim)
            {
                if (pacArea(current) > pacArea(largest))
                {
                    largest = current;
                }
            }
        }
    }

    return largest;
}

//
// pacSelect
// select a texture subject to constraints.
// prefer largest manhattan size
//
liflist_t* pacSelect(sdword maxWidth, sdword maxHeight)
{
    liflist_t* selection;
    liflist_t* current;
    sdword currentSize, selectionSize;

    sdword constraintMax, constraintMin;
    sdword dimMax, dimMin;

    //find largest manhattan size

    constraintMax = MAX(maxWidth, maxHeight);
    constraintMin = MIN(maxWidth, maxHeight);

    selection = NULL;

    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        current = &(*i);

        //continue if already packed
        if (current->flags & PACF_Packed)
        {
            continue;
        }

        //continue if too large
        dimMax = MAX(current->header->width, current->header->height);
        dimMin = MIN(current->header->width, current->header->height);
        if (dimMax > constraintMax ||
            dimMin > constraintMin)
        {
            continue;
        }

        //set selection
        if (selection == NULL)
        {
            selection = current;
        }
        else
        {
            //compare manhattan sizes
            currentSize = current->header->width + current->header->height;
            selectionSize = selection->header->width + selection->header->height;
            if (currentSize > selectionSize)
            {
                //larger manhattan size
                selection = current;
            }
            else if (currentSize == selectionSize)
            {
                if ((current->flags & PACF_Square) &&
                    !(selection->flags & PACF_Square))
                {
                    //current is square, selection is not
                    selection = current;
                }
            }
        }
    }

    return selection;
}

//
// pacRotateRGBA
// transpose an RGBA LiF
//
void pacRotateRGBA(liflist_t* lif)
{
    sdword y, x;
    color* data;
    color* source;
    ubyte* team0;
    ubyte* team1;

    source = (color*)lif->header->data;

    data = new color[lif->header->width * lif->header->height];
    team0 = new ubyte[lif->header->width * lif->header->height];
    team1 = new ubyte[lif->header->width * lif->header->height];

    //data
    for (y = 0; y < lif->header->width; y++)
    {
        for (x = 0; x < lif->header->height; x++)
        {
            PIXEL(data, y, x, lif->header->width) = PIXEL(source, x, y, lif->header->height);
        }
    }

    //teamcolors
    for (y = 0; y < lif->header->width; y++)
    {
        for (x = 0; x < lif->header->height; x++)
        {
            if (lif->header->teamEffect0 != NULL)
            {
                PIXEL(team0, y, x, lif->header->width) = PIXEL(lif->header->teamEffect0, x, y, lif->header->height);
            }
            if (lif->header->teamEffect1 != NULL)
            {
                PIXEL(team1, y, x, lif->header->width) = PIXEL(lif->header->teamEffect1, x, y, lif->header->height);
            }
        }
    }

    memcpy(source, data, sizeof(color) * lif->header->width * lif->header->height);
    if (lif->header->teamEffect0 != NULL)
    {
        memcpy(lif->header->teamEffect0, team0, lif->header->width * lif->header->height);
    }
    if (lif->header->teamEffect1 != NULL)
    {
        memcpy(lif->header->teamEffect1, team1, lif->header->width * lif->header->height);
    }

    delete [] data;
    delete [] team0;
    delete [] team1;
}

//
// pacRotate
// transpose a LiF
//
void pacRotate(liflist_t* lif)
{
    sdword y, x;
    ubyte* data;

    if (!(lif->header->flags & TRF_Paletted))
    {
        pacRotateRGBA(lif);
        return;
    }

    data = new ubyte[lif->header->width * lif->header->height];

    for (y = 0; y < lif->header->width; y++)
    {
        for (x = 0; x < lif->header->height; x++)
        {
            PIXEL(data, y, x, lif->header->width) = PIXEL(lif->header->data, x, y, lif->header->height);
        }
    }

    memcpy(lif->header->data, data, lif->header->width * lif->header->height);

    delete [] data;
}

//
// pacFlagRotated
// flag a LiF as having been rotated,
// transpose dimensions
//
void pacFlagRotated(liflist_t* lif)
{
    sdword temp;

    temp = lif->header->width;
    lif->header->width = lif->header->height;
    lif->header->height = temp;

    if (lif->flags & PACF_Rotated)
    {
        lif->flags &= ~PACF_Rotated;
    }
    else
    {
        lif->flags |= PACF_Rotated;
    }
}

//
//does NOT update offsets in page
//
void pacVerticalSubStrip(lifpage* page, sdword startX, sdword startY, sdword& stripWidth, sdword& stripHeight, sdword maxWidth, sdword maxHeight)
{
    sdword xOffset, yOffset;
    sdword heightLeft;
    liflist_t* lif;

    //get offsets
    xOffset = startX;
    yOffset = startY;

    //select texture
    lif = pacSelect(maxWidth, maxHeight);

    //get anything?
    if (lif == NULL)
    {
        return;
    }

    //ensure horizontal dominance
    if (lif->header->height > lif->header->width)
    {
        pacFlagRotated(lif);
    }

    //rotate if out of bounds
    if (lif->header->width > maxWidth ||
        lif->header->height > maxHeight)
    {
        pacFlagRotated(lif);
    }

    //place texture
    PLACE_LIF(lif, page, xOffset, yOffset);

    //set strip dimensions
    stripWidth = lif->header->width;
    stripHeight = maxHeight;

    //update offsets
    yOffset += lif->header->height;
    heightLeft = maxHeight - lif->header->height;

    //already completed?
    if (heightLeft <= 0)
    {
        return;
    }

    for (;;)
    {
        //select texture
        lif = pacSelect(stripWidth, heightLeft);

        //get anything?
        if (lif == NULL)
        {
            return;
        }

        //ensure horizontal dominance
        if (lif->header->height > lif->header->width)
        {
            pacFlagRotated(lif);
        }

        //rotate if out of bounds
        if (lif->header->width > stripWidth ||
            lif->header->height > heightLeft)
        {
            pacFlagRotated(lif);
        }

        //place texture
        PLACE_LIF(lif, page, xOffset, yOffset);

        //check for horizontal motion
        if (lif->header->width < stripWidth)
        {
            sdword horizMaxWidth, horizMaxHeight;
            sdword horizWidth, horizHeight;

            horizMaxWidth = stripWidth - lif->header->width;
            horizMaxHeight = stripHeight;
            pacHorizontalSubStrip(page, xOffset + lif->header->width, yOffset, horizWidth, horizHeight, horizMaxWidth, horizMaxHeight);
        }

        //update offsets
        yOffset += lif->header->height;
        heightLeft -= lif->header->height;

        //completed?
        if (heightLeft <= 0)
        {
            return;
        }
    }
}

//
//does NOT update offsets in page
//
void pacHorizontalSubStrip(lifpage* page, sdword startX, sdword startY, sdword& stripWidth, sdword& stripHeight, sdword maxWidth, sdword maxHeight)
{
    sdword xOffset, yOffset;
    sdword widthLeft;
    liflist_t* lif;

    //get offsets
    xOffset = startX;
    yOffset = startY;

    //select left-most texture
    lif = pacSelect(maxWidth, maxHeight);

    //get anything?
    if (lif == NULL)
    {
        return;
    }

    //ensure horizontal dominance
    if (lif->header->height > lif->header->width)
    {
        pacFlagRotated(lif);
    }

    //rotate if out of bounds
    if (lif->header->width > maxWidth ||
        lif->header->height > maxHeight)
    {
        pacFlagRotated(lif);
    }

    //place texture
    PLACE_LIF(lif, page, xOffset, yOffset);

    //set strip dimensions
    stripWidth = maxWidth;              //we'll be filling as much as we can horizontally
    stripHeight = lif->header->height;  //we'll only be filling to this vertical position

    //update offsets
    xOffset += lif->header->width;
    widthLeft = maxWidth - lif->header->width;

    //already completed a horizontal pack?
    if (widthLeft <= 0)
    {
        return;
    }

    for (;;)
    {
        //select a texture
        lif = pacSelect(widthLeft, stripHeight);

        //get anything?
        if (lif == NULL)
        {
            return;
        }

        //ensure horizontal dominance
        if (lif->header->height > lif->header->width)
        {
            pacFlagRotated(lif);
        }

        //rotate if out of bounds
        if (lif->header->width > widthLeft ||
            lif->header->height > stripHeight)
        {
            pacFlagRotated(lif);
        }

        //place texture
        PLACE_LIF(lif, page, xOffset, yOffset);

        //check for vertical motion
        if (lif->header->height < stripHeight)
        {
            sdword vertMaxWidth, vertMaxHeight;
            sdword vertWidth, vertHeight;

            vertMaxWidth = lif->header->width;
            vertMaxHeight = stripHeight - lif->header->height;
            pacVerticalSubStrip(page, xOffset, yOffset + lif->header->height, vertWidth, vertHeight, vertMaxWidth, vertMaxHeight);
        }

        //update offsets
        xOffset += lif->header->width;
        widthLeft -= lif->header->width;

        //completed?
        if (widthLeft <= 0)
        {
            return;
        }
    }
}

//
//does NOT update offsets in page
//
void pacVerticalStrip(lifpage* page, sdword& stripWidth, sdword& stripHeight, sdword maxWidth, sdword maxHeight)
{
    sdword xOffset, yOffset;
    sdword heightLeft;
    liflist_t* lif;

    //get offsets
    xOffset = page->xOffset;
    yOffset = page->yOffset;

    //select the largest texture w/ largest dim for upper left
    lif = pacSelectFirst(maxWidth, maxHeight);

    //get anything?
    if (lif == NULL)
    {
        return;
    }

    //ensure horizontal dominance
    if (lif->header->height > lif->header->width)
    {
        pacFlagRotated(lif);
    }

    //rotate if out of bounds
    if (lif->header->width > maxWidth ||
        lif->header->height > maxHeight)
    {
        pacFlagRotated(lif);
    }

    //place texture
    PLACE_LIF(lif, page, xOffset, yOffset);

    //set strip dimensions
    stripWidth = lif->header->width;
    stripHeight = maxHeight;

    //update offsets
    yOffset += lif->header->height;

    //set distance to travel
    heightLeft = maxHeight - lif->header->height;

    //now pack horizontally to stripWidth, stripHeight
    for (;;)
    {
        sdword hWidth, hHeight;

        //page, x offset, y offset, actual width, actual height, max width, max height
        pacHorizontalSubStrip(page, xOffset, yOffset, hWidth, hHeight, stripWidth, heightLeft);

        //update position
        yOffset += hHeight;

        //update distance
        heightLeft -= hHeight;
        if (heightLeft <= 0)
        {
            //finished moving vertically
            break;
        }
    }
}

//
// pacTooMuchWastedSpace
// decide whether there's "too much" wasted
// space on a texture page
//
bool pacTooMuchWastedSpace(lifpage* page)
{
    real32 pageArea, lifArea;
    real32 ratio;

    pageArea = page->width * page->height;

    lifArea = 0.0f;
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent == page)
        {
            lifArea += (*i).header->width * (*i).header->height;
        }
    }

    ratio = lifArea / pageArea;

    if ((page->width + page->height) > LARGE_MANHATTAN)
    {
        //"large" page
        if (ratio <= THRESH_LARGE)
        {
            return true;
        }
    }
    else if ((page->width + page->height) > SMALL_MANHATTAN)
    {
        //"small" page
        if (ratio <= THRESH_SMALL)
        {
            return true;
        }
    }

    return false;
}

//
// pacFixupBadness
// remove errors
//
void pacFixupBadness(lifpage* page)
{
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).parent != page)
        {
            //not ours
            continue;
        }

        if ((*i).packedX > page->width ||
            (*i).packedY > page->height ||
            ((*i).packedX + (*i).header->width) > page->width ||
            ((*i).packedY + (*i).header->height) > page->height)
        {
            (*i).flags &= ~PACF_Packed;
            if ((*i).flags & PACF_Rotated)
            {
                pacFlagRotated(&(*i));
            }
        }
    }
}

bool pacPack(bool alpha)
{
    //repeatedly pack vertically until no more strips will fit,
    //or we run out of textures

    lifpage* page;
    sdword   width, height, maxHeight;
    sdword   pageWidth, pageHeight;

    if (!pacTexturesRemaining())
    {
        return true;
    }

    page = new lifpage;
    page->flags = alpha ? PACF_Alpha : PACF_Paletted;
    page->width = 256;
    page->height = 256;
  
  REPACK:
    //begin at upper left
    page->xOffset = 0;
    page->yOffset = 0;

    for (;;)
    {
        //return width, height actual dim
        pacVerticalStrip(page, width, height, page->width - page->xOffset, page->height);

        //reset offsets
        page->xOffset += width;
        page->yOffset = 0;
        if (page->xOffset >= page->width)
        {
            break;
        }

        //check for remaining textures
        if (!pacTexturesRemaining())
        {
            break;
        }
    }

    //crop page
    pageWidth = page->width;
    pageHeight = page->height;
    if (pacNextPowerOf2(page->xOffset) < page->width)
    {
        page->width = pacNextPowerOf2(page->xOffset);
    }
    maxHeight = pacHeightOfPage(page);
    if (pacNextPowerOf2(maxHeight) < page->height)
    {
        page->height = pacNextPowerOf2(maxHeight);
    }

    //calculate wasted space and determine whether
    //a repacking need occur.
    //ie. unpack all textures on this page, delete the page,
    //create a new page of smaller size, pack
    if (pacTooMuchWastedSpace(page))
    {
        //scan texture list
        for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
        {
            //one of ours?
            if ((*i).parent != page)
            {
                continue;
            }

            //un-rotate
            if ((*i).flags & PACF_Rotated)
            {
                sdword temp;

                (*i).flags &= ~PACF_Rotated;
                temp = (*i).header->width;
                (*i).header->width = (*i).header->height;
                (*i).header->height = temp;
            }

            //un-pack
            (*i).flags &= ~PACF_Packed;

            //un-parent
            (*i).parent = NULL;
        }

        if (page->width >= page->height)
        {
            page->width = pageWidth / 2;
            page->height = pageHeight;
        }
        else
        {
            page->width = pageWidth;
            page->height = pageHeight / 2;
        }
        goto REPACK;
    }

    //fixup bad textures
    pacFixupBadness(page);

    //add page to pagelist
    lifpagelist.push_back(page);

    //? delete our copy of the page ?

    //return false if more packing needs to occur
    return !pacTexturesRemaining();
}

//
// pacCleanup
// post-packing procedures
//
void pacCleanup(void)
{
    //rotate textures flagged as rotated
    for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
    {
        if ((*i).flags & PACF_Rotated)
        {
            pacRotate(&(*i));
        }
    }

    //error checking
    for (list_lifpage::iterator page = lifpagelist.begin(); page != lifpagelist.end(); ++page)
    {
        for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
        {
            if ((*i).parent != (*page))
            {
                //not our LiF
                continue;
            }

            if ((*i).packedX > (*page)->width ||
                (*i).packedY > (*page)->height ||
                ((*i).packedX + (*i).header->width) > (*page)->width ||
                ((*i).packedY + (*i).header->height) > (*page)->height)
            {
                //out of bounds
                assert(false);
            }
        }
    }
}

//
// pacOutputPageContents
// verbose output about page contents
//
void pacOutputPageContents(void)
{
    sdword pageNum = 0;
    for (list_lifpage::iterator page = lifpagelist.begin(); page != lifpagelist.end(); ++page, ++pageNum)
    {
        fprintf(stderr, "\npage %d (%d.%d) %s\n",
                pageNum, (*page)->width, (*page)->height,
                ((*page)->flags & PACF_Alpha) ? "alpha" : "paletted");
        for (list_lif::iterator i = liflist.begin(); i != liflist.end(); ++i)
        {
            if ((*i).parent != (*page))
            {
                //not our LiF
                continue;
            }

            fprintf(stderr, "  %3d %3d  %s [%dx%d]\n",
                    (*i).packedX,
                    (*i).packedY,
                    lifShortName((*i).filename),
                    (*i).header->width,
                    (*i).header->height);
        }
    }
}

//
// pacNamePages
// generate filenames for the pages
//
void pacNamePages(void)
{
    char name[] = "pagen";
    sdword pageNum;

    if (pacOutputPages)
    {
        pacOutputPageContents();
    }

    pageNum = 0;
    for (list_lifpage::iterator page = lifpagelist.begin(); page != lifpagelist.end(); ++page)
    {
        name[4] = '0' + pageNum;
        strcpy((*page)->name, name);
        pageNum++;
    }
}
