/**********************************************************************
        C Implementation of Wu's Color Quantizer (v. 2)
        (see Graphics Gems vol. II, pp. 126-133)

Author:	Xiaolin Wu
    Dept. of Computer Science
    Univ. of Western Ontario
    London, Ontario N6A 5B7
    wu@csd.uwo.ca

Algorithm: Greedy orthogonal bipartition of RGB space for variance
       minimization aided by inclusion-exclusion tricks.
       For speed no nearest neighbor search is done. Slightly
       better performance can be expected by more sophisticated
       but more expensive versions.

The author thanks Tom Lane at Tom_Lane@G.GP.CS.CMU.EDU for much of
additional documentation and a cure to a previous bug.

Free to distribute, comments and suggestions are appreciated.

NB: most modifications by Luke Moloney are for beautification and
    efficiency of inputs/outputs.
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "memory.h"
#include "color.h"
#include "quantize.h"

#define NUMBER_COLORS      (256)
#define MAXCOLOR	256
#define	RED	2
#define	GREEN	1
#define BLUE	0
#define q3D(h, a, b, c)   ((h)[(a) * 33 * 33 + (b) * 33 + (c)])
//#define q3D(h, a, b, c)   ((h)[(a)][b][c])

struct box
{
    int r0;          /* min value, exclusive */
    int r1;          /* max value, inclusive */
    int g0;
    int g1;
    int b0;
    int b1;
    int vol;
};

/* Histogram is in elements 1..HISTSIZE along each axis,
 * element 0 is for base or marginal value
 * NB: these must start out 0!
 */

real32 *qHistogram;
sdword *qHistoWeight, *qHistoRed, *qHistoGreen, *qHistoBlue;
sdword qImageSize; /*image size*/
int     qCLUTSize;    /*color look-up table size*/
uword *qIndexBuffer;
color *qRGBBuffer;
color *qPalette;
sdword qCalcIndices = TRUE;
udword *qColorUsageCount;

quantdata qQuantizeQueue[Q_QueueLength];
sdword qQuantizeQueueIndex = 0;

/*-----------------------------------------------------------------------------
    Name        : Hist3d
    Description : build 3-D color histogram of counts, r/g/b, c^2
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void Hist3d(long int *vwt, long int *vmr, long int *vmg, long int *vmb, float *m2)
{
    register int ind, r, g, b;
    int      inr, ing, inb, table[256];
    register long int i;

    for (i=0; i<256; ++i) table[i]=i*i;
    for (i=0; i<qImageSize; ++i)
    {
        r = colRed(qRGBBuffer[i]); g = colGreen(qRGBBuffer[i]); b = colBlue(qRGBBuffer[i]);
        inr=(r>>3)+1;
        ing=(g>>3)+1;
        inb=(b>>3)+1;
        ind=(inr<<10)+(inr<<6)+inr+(ing<<5)+ing+inb;
        if (qCalcIndices)
        {
            qIndexBuffer[i] = ind;
        }
        /*[inr][ing][inb]*/
        ++vwt[ind];
        vmr[ind] += r;
        vmg[ind] += g;
        vmb[ind] += b;
        m2[ind] += (float)(table[r]+table[g]+table[b]);
    }
}

/* At conclusion of the histogram step, we can interpret
 *   qHistoWeight[r][g][b] = sum over voxel of P(c)
 *   qHistoRed[r][g][b] = sum over voxel of r*P(c)  ,  similarly for qHistoGreen, qHistoBlue
 *   m2[r][g][b] = sum over voxel of c^2*P(c)
 * Actually each of these should be divided by 'size' to give the usual
 * interpretation of P() as ranging from 0 to 1, but we needn't do that here.
 */

/* We now convert histogram into moments so that we can rapidly calculate
 * the sums of the above quantities over any desired box.
 */


void
M3d(vwt, vmr, vmg, vmb, m2) /* compute cumulative moments. */
long int *vwt, *vmr, *vmg, *vmb;
float   *m2;
{
    register unsigned short int ind1, ind2;
    register unsigned char i, r, g, b;
    long int line, line_r, line_g, line_b,
    area[33], area_r[33], area_g[33], area_b[33];
    real32 line2, area2[33];

    for (r=1; r<=32; ++r)
    {
        for (i=0; i<=32; ++i)
        {
            area2[i] = 0.0f;
            area[i]=area_r[i]=area_g[i]=area_b[i]=0;
        }
        for (g=1; g<=32; ++g)
        {
            line2 = 0.0f;
            line = line_r = line_g = line_b = 0;
            for (b=1; b<=32; ++b)
            {
                ind1 = (r<<10) + (r<<6) + r + (g<<5) + g + b; /* [r][g][b] */
                line += vwt[ind1];
                line_r += vmr[ind1];
                line_g += vmg[ind1];
                line_b += vmb[ind1];
                line2 += m2[ind1];
                area[b] += line;
                area_r[b] += line_r;
                area_g[b] += line_g;
                area_b[b] += line_b;
                area2[b] += line2;
                ind2 = ind1 - 1089; /* [r-1][g][b] */
                vwt[ind1] = vwt[ind2] + area[b];
                vmr[ind1] = vmr[ind2] + area_r[b];
                vmg[ind1] = vmg[ind2] + area_g[b];
                vmb[ind1] = vmb[ind2] + area_b[b];
                m2[ind1] = m2[ind2] + area2[b];
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : qVolume
    Description : Compute sum over a box of any given statistic
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
//sdword qVolume(struct box *cube, sdword mmt[33][33][33])
sdword qVolume(struct box *cube, sdword *mmt)
{
    sdword retVal;
    retVal = q3D(mmt, cube->r1, cube->g1, cube->b1);
    retVal -= q3D(mmt, cube->r1, cube->g1, cube->b0);
    retVal -= q3D(mmt, cube->r1, cube->g0, cube->b1);
    retVal += q3D(mmt, cube->r1, cube->g0, cube->b0);
    retVal -= q3D(mmt, cube->r0, cube->g1, cube->b1);
    retVal += q3D(mmt, cube->r0, cube->g1, cube->b0);
    retVal += q3D(mmt, cube->r0, cube->g0, cube->b1);
    retVal -= q3D(mmt, cube->r0, cube->g0, cube->b0);
    return(retVal);
}

/* The next two routines allow a slightly more efficient calculation
 * of qVolume() for a proposed subbox of a given box.  The sum of Top()
 * and Bottom() is the qVolume() of a subbox split in the given direction
 * and with the specified new upper bound.
 */

/* Compute part of qVolume(cube, mmt) that doesn't depend on r1, g1, or b1 */
/* (depending on dir) */
//long int Bottom(struct box *cube, ubyte dir, sdword mmt[33][33][33])
long int Bottom(struct box *cube, ubyte dir, sdword *mmt)
{
    switch (dir)
    {
        case RED:
            return( -q3D(mmt, cube->r0, cube->g1, cube->b1)
                    +q3D(mmt, cube->r0, cube->g1, cube->b0)
                    +q3D(mmt, cube->r0, cube->g0, cube->b1)
                    -q3D(mmt, cube->r0, cube->g0, cube->b0) );
            break;
        case GREEN:
            return( -q3D(mmt, cube->r1, cube->g0, cube->b1)
                    +q3D(mmt, cube->r1, cube->g0, cube->b0)
                    +q3D(mmt, cube->r0, cube->g0, cube->b1)
                    -q3D(mmt, cube->r0, cube->g0, cube->b0) );
            break;
        case BLUE:
            return( -q3D(mmt, cube->r1, cube->g1, cube->b0)
                    +q3D(mmt, cube->r1, cube->g0, cube->b0)
                    +q3D(mmt, cube->r0, cube->g1, cube->b0)
                    -q3D(mmt, cube->r0, cube->g0, cube->b0) );
            break;
    }
}


/* Compute remainder of qVolume(cube, mmt), substituting pos for */
/* r1, g1, or b1 (depending on dir) */
//long int Top(struct box *cube, ubyte dir, sdword pos, sdword mmt[33][33][33])
long int Top(struct box *cube, ubyte dir, sdword pos, sdword *mmt)
{
    switch (dir)
    {
        case RED:
            return( q3D(mmt, pos, cube->g1, cube->b1)
                    -q3D(mmt, pos, cube->g1, cube->b0)
                    -q3D(mmt, pos, cube->g0, cube->b1)
                    +q3D(mmt, pos, cube->g0, cube->b0) );
            break;
        case GREEN:
            return( q3D(mmt, cube->r1, pos, cube->b1)
                    -q3D(mmt, cube->r1, pos, cube->b0)
                    -q3D(mmt, cube->r0, pos, cube->b1)
                    +q3D(mmt, cube->r0, pos, cube->b0) );
            break;
        case BLUE:
            return( q3D(mmt, cube->r1, cube->g1, pos)
                    -q3D(mmt, cube->r1, cube->g0, pos)
                    -q3D(mmt, cube->r0, cube->g1, pos)
                    +q3D(mmt, cube->r0, cube->g0, pos) );
            break;
    }
}


float Var(struct box *cube)
/* Compute the weighted variance of a box */
/* NB: as with the raw statistics, this is really the variance * size */
{
    real32 dr, dg, db, xx;

    dr = (real32)qVolume(cube, qHistoRed);
    dg = (real32)qVolume(cube, qHistoGreen);
    db = (real32)qVolume(cube, qHistoBlue);
    xx =  q3D(qHistogram, cube->r1, cube->g1, cube->b1)
          -q3D(qHistogram, cube->r1, cube->g1, cube->b0)
          -q3D(qHistogram, cube->r1, cube->g0, cube->b1)
          +q3D(qHistogram, cube->r1, cube->g0, cube->b0)
          -q3D(qHistogram, cube->r0, cube->g1, cube->b1)
          +q3D(qHistogram, cube->r0, cube->g1, cube->b0)
          +q3D(qHistogram, cube->r0, cube->g0, cube->b1)
          -q3D(qHistogram, cube->r0, cube->g0, cube->b0);

    return( xx - (dr*dr+dg*dg+db*db)/(float)qVolume(cube,qHistoWeight) );
}

/* We want to minimize the sum of the variances of two subboxes.
 * The sum(c^2) terms can be ignored since their sum over both subboxes
 * is the same (the sum for the whole box) no matter where we split.
 * The remaining terms have a minus sign in the variance formula,
 * so we drop the minus sign and MAXIMIZE the sum of the two terms.
 */


float Maximize(cube, dir, first, last, cut,
               whole_r, whole_g, whole_b, whole_w)
struct box *cube;
unsigned char dir;
int first, last, *cut;
long int whole_r, whole_g, whole_b, whole_w;
{
    register long int half_r, half_g, half_b, half_w;
    long int base_r, base_g, base_b, base_w;
    register int i;
    register float temp, max;

    base_r = Bottom(cube, dir, qHistoRed);
    base_g = Bottom(cube, dir, qHistoGreen);
    base_b = Bottom(cube, dir, qHistoBlue);
    base_w = Bottom(cube, dir, qHistoWeight);
    max = 0.0f;
    *cut = -1;
    for (i=first; i<last; ++i)
    {
        half_r = base_r + Top(cube, dir, i, qHistoRed);
        half_g = base_g + Top(cube, dir, i, qHistoGreen);
        half_b = base_b + Top(cube, dir, i, qHistoBlue);
        half_w = base_w + Top(cube, dir, i, qHistoWeight);
        /* now half_x is sum over lower half of box, if split at i */
        if (half_w == 0)
        {      /* subbox could be empty of pixels! */
            continue;             /* never split into an empty box */
        }
        else
            temp = ((float)half_r*half_r + (float)half_g*half_g +
                    (float)half_b*half_b)/half_w;

        half_r = whole_r - half_r;
        half_g = whole_g - half_g;
        half_b = whole_b - half_b;
        half_w = whole_w - half_w;
        if (half_w == 0)
        {      /* subbox could be empty of pixels! */
            continue;             /* never split into an empty box */
        }
        else
            temp += ((float)half_r*half_r + (float)half_g*half_g +
                     (float)half_b*half_b)/half_w;

        if (temp > max)
        {max=temp; *cut=i;}
    }
    return(max);
}

int
Cut(set1, set2)
struct box *set1, *set2;
{
    unsigned char dir;
    int cutr, cutg, cutb;
    float maxr, maxg, maxb;
    long int whole_r, whole_g, whole_b, whole_w;

    whole_r = qVolume(set1, qHistoRed);
    whole_g = qVolume(set1, qHistoGreen);
    whole_b = qVolume(set1, qHistoBlue);
    whole_w = qVolume(set1, qHistoWeight);

    maxr = Maximize(set1, RED, set1->r0+1, set1->r1, &cutr,
                    whole_r, whole_g, whole_b, whole_w);
    maxg = Maximize(set1, GREEN, set1->g0+1, set1->g1, &cutg,
                    whole_r, whole_g, whole_b, whole_w);
    maxb = Maximize(set1, BLUE, set1->b0+1, set1->b1, &cutb,
                    whole_r, whole_g, whole_b, whole_w);

    if ( (maxr>=maxg)&&(maxr>=maxb) )
    {
        dir = RED;
        if (cutr < 0) return 0; /* can't split the box */
    }
    else
        if ( (maxg>=maxr)&&(maxg>=maxb) )
        dir = GREEN;
    else
        dir = BLUE;

    set2->r1 = set1->r1;
    set2->g1 = set1->g1;
    set2->b1 = set1->b1;

    switch (dir)
    {
        case RED:
            set2->r0 = set1->r1 = cutr;
            set2->g0 = set1->g0;
            set2->b0 = set1->b0;
            break;
        case GREEN:
            set2->g0 = set1->g1 = cutg;
            set2->r0 = set1->r0;
            set2->b0 = set1->b0;
            break;
        case BLUE:
            set2->b0 = set1->b1 = cutb;
            set2->r0 = set1->r0;
            set2->g0 = set1->g0;
            break;
    }
    set1->vol=(set1->r1-set1->r0)*(set1->g1-set1->g0)*(set1->b1-set1->b0);
    set2->vol=(set2->r1-set2->r0)*(set2->g1-set2->g0)*(set2->b1-set2->b0);
    return 1;
}


void Mark(struct box *cube, sdword label, ubyte *tag)
{
    register int r, g, b;

    for (r=cube->r0+1; r<=cube->r1; ++r)
        for (g=cube->g0+1; g<=cube->g1; ++g)
            for (b=cube->b0+1; b<=cube->b1; ++b)
                tag[(r<<10) + (r<<6) + r + (g<<5) + g + b] = (ubyte)label;
}

/*-----------------------------------------------------------------------------
    Name        : quantizeMain
    Description : Main quantization routine
    Inputs      : nColors - number of colors to quantize to
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void quantizeMain(sdword nColors)
{
    struct box  cube[MAXCOLOR];
    unsigned char   *tag;
    int     next;
    register long int   i, weight;
    register int    k;
    float       vv[MAXCOLOR], temp;
    udword red, green, blue;

    /* input R,G,B components into Ir, Ig, Ib;
       set size to width*height */

//    qCLUTSize = NUMBER_COLORS;
    qCLUTSize = nColors;

    Hist3d(qHistoWeight, qHistoRed, qHistoGreen, qHistoBlue, qHistogram);
#if Q_VERBOSE_LEVEL >= 1
    dbgMessagef("\nquantizeMain: Histogram done");
#endif
//    Hist3d(&qHistoWeight[0][0][0], &qHistoRed[0][0][0], &qHistoGreen[0][0][0], &qHistoBlue[0][0][0], &qHistogram[0][0][0]);
#if Q_VERBOSE_LEVEL >= 1
    dbgMessagef("\nquantizeMain: Histogram done");
#endif
    M3d(qHistoWeight, qHistoRed, qHistoGreen, qHistoBlue, qHistogram);
#if Q_VERBOSE_LEVEL >= 1
    dbgMessagef("\nquantizeMain: Moments done");
#endif

    cube[0].r0 = cube[0].g0 = cube[0].b0 = 0;
    cube[0].r1 = cube[0].g1 = cube[0].b1 = 32;
    next = 0;
    for (i=1; i<qCLUTSize; ++i)
    {
        if (Cut(&cube[next], &cube[i]))
        {
            /* volume test ensures we won't try to cut one-cell box */
            vv[next] = (cube[next].vol>1) ? Var(&cube[next]) : 0.0f;
            vv[i] = (cube[i].vol>1) ? Var(&cube[i]) : 0.0f;
        }
        else
        {
            vv[next] = 0.0f;   /* don't try to split this box again */
            i--;              /* didn't create box i */
        }
        next = 0; temp = vv[0];
        for (k=1; k<=i; ++k)
            if (vv[k] > temp)
            {
                temp = vv[k]; next = k;
            }
        if (temp <= 0.0f)
        {
            qCLUTSize = i+1;
#if Q_VERBOSE_LEVEL >= 1
            dbgMessagef("\nquantizeMain: Only got %d boxes", qCLUTSize);
#endif
            break;
        }
    }
#if Q_VERBOSE_LEVEL >= 1
    dbgMessagef("\nquantizeMain: Partition done");
#endif

    /* the space for array qHistogram can be freed now */

    tag = memAlloc(33*33*33, "Histogram stuff", 0);

    for (k=0; k<qCLUTSize; ++k)
    {
        Mark(&cube[k], k, tag);
        weight = qVolume(&cube[k], qHistoWeight);
        if (weight)
        {
            red = qVolume(&cube[k], qHistoRed) / weight;
            green = qVolume(&cube[k], qHistoGreen) / weight;
            blue = qVolume(&cube[k], qHistoBlue) / weight;
            qPalette[k] = colRGB(red, green, blue);
        }
        else
        {
            qPalette[k] = colBlack;
        }
    }

    if (qCalcIndices)
    {
        for (i=0; i<qImageSize; ++i)
        {
            qIndexBuffer[i] = tag[qIndexBuffer[i]];
            qColorUsageCount[qIndexBuffer[i]]++;
        }
    }
    memFree(tag);
}

/*-----------------------------------------------------------------------------
    Name        : qRGBImageQuantize
    Description : Quantize a RGB image to index-mode using the X Wu algorithm.
    Inputs      : destIndex - where the image is to be stored.
                  destPalette - where the computed palette is to be stored
                    (256 in length).
                  source - source RGB image
                  width,height - size of source image, and hence dest image.
                  flags - controls operation of quantizing including:
                    Q_SkipIndices - skip index pixel calculation
    Outputs     : Computes and saves palette to destPalette and corresponding
                    image to destIndex.
    Return      : Number of entries in new palette (typically 256 for complex images).
----------------------------------------------------------------------------*/
sdword qRGBImageQuantize(ubyte *destIndex, color *destPalette, color *source, sdword width, sdword height, sdword flags)
{
    sdword index;

    qCalcIndices = !bitTest(flags, Q_SkipIndices);
    qImageSize = width * height;
    if (qCalcIndices)
    {
        qIndexBuffer = memAlloc(qImageSize * 2, "16-bit index image temp", 0);
    }
    qRGBBuffer = source;
    qPalette = destPalette;

    //histograms can be big; let's allocate them dynamically so as not to waste
    //static storage
    qHistogram   = memAlloc(sizeof(real32) * 33 * 33 * 33, "qHistogram   ", 0);
    qHistoWeight = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoWeight ", 0);
    qHistoRed    = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoRed    ", 0);
    qHistoGreen  = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoGreen  ", 0);
    qHistoBlue   = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoBlue   ", 0);
    //because there is the assumption that the static data is all zeros,
    //we must now clear out all these dynamic arrays
    memset(qHistogram, 0, sizeof(real32) * 33 * 33 * 33);
    memset(qHistoWeight, 0, sizeof(sdword) * 33 * 33 * 33);
    memset(qHistoRed, 0, sizeof(sdword) * 33 * 33 * 33);
    memset(qHistoGreen, 0, sizeof(sdword) * 33 * 33 * 33);
    memset(qHistoBlue, 0, sizeof(sdword) * 33 * 33 * 33);

    quantizeMain(NUMBER_COLORS);

    memFree(qHistoBlue  );
    memFree(qHistoGreen );
    memFree(qHistoRed   );
    memFree(qHistoWeight);
    memFree(qHistogram  );

    if (qCalcIndices)
    {                                                       //build 8-bit indexed images
        for (index = 0; index < qImageSize; index++)
        {
            destIndex[index] = (ubyte)qIndexBuffer[index];
        }
        memFree(qIndexBuffer);                              //free 16-bit index array
    }
    return(qCLUTSize);
}

/*-----------------------------------------------------------------------------
    Name        : qQuantizeReset
    Description : Reset the quantization queue
    Inputs      : void
    Outputs     : sets qQuantizeQueueIndex to zero
    Return      : void
----------------------------------------------------------------------------*/
void qQuantizeReset(void)
{
    qQuantizeQueueIndex = 0;
}

/*-----------------------------------------------------------------------------
    Name        : qRGBImageAdd
    Description : Adds an image to the quantization queue.
    Inputs      : same as for single-image quantization except no palette
    Outputs     : saves the parameters into qQuantizeQueue
    Return      : void
----------------------------------------------------------------------------*/
void qRGBImageAdd(ubyte *destIndex, color *source, sdword width, sdword height, ubyte *teamColor)
{
#if Q_ERROR_CHECKING
    if (qQuantizeQueueIndex >= Q_QueueLength)
    {
        dbgFatalf(DBG_Loc, "quantization image queue too short: %d", qQuantizeQueueIndex);
    }
#endif
    qQuantizeQueue[qQuantizeQueueIndex].width = width;
    qQuantizeQueue[qQuantizeQueueIndex].height = height;
    qQuantizeQueue[qQuantizeQueueIndex].RGBSource = source;
    qQuantizeQueue[qQuantizeQueueIndex].indexDest = destIndex;
    qQuantizeQueue[qQuantizeQueueIndex].teamColor = teamColor;
    qQuantizeQueueIndex++;
}

/*-----------------------------------------------------------------------------
    Name        : qReductionSortCB
    Description : qsort callback for use in preparing palette reduction table
    Inputs      : standard qsort
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
typedef struct
{
    sdword index;
    sdword usageCount;
}
qtablesortstruct;
int qReductionSortCB(const void *p0, const void *p1)
{
    return(((qtablesortstruct *)p1)->usageCount - ((qtablesortstruct *)p0)->usageCount);
}

/*-----------------------------------------------------------------------------
    Name        : qPaletteReductionTableCreate
    Description : Creates a palette reduction table and a new palette.
    Inputs      : destTable - out parameter for the reduction table.
                  destPalette - out parameter for the new partial palette.
                  base - index of the base of the new partial palette.
                  length - length of the new partial palette
                  palette - initial palette
                  paletteLength - length of input palette
                  colorUsageCount - usage count for the colors in palette
    Outputs     : Fills in the destTable and destPalette as such:
        destTable - the first paletteLength entries will be biased indices into
            destPalette.  They will be in the range of base..base + length.
        destPalette - entries base..base + length will be the 'most important'
            colors in the input palette.  These indices are referenced by the
            destTable.  This is one area where this algorithm could be improved
            if it mangles the color space too much.
    Return      : void
----------------------------------------------------------------------------*/
void qPaletteReductionTableCreate(ubyte *destTable, color *destPalette,
                                  sdword base, sdword length, color *palette,
                                  sdword paletteLength, udword *colorUsageCount)
{
    sdword index;
    qtablesortstruct sortList[NUMBER_COLORS];

    dbgAssert(paletteLength <= NUMBER_COLORS);              //verify some assumptions
    dbgAssert(base + length <= NUMBER_COLORS);

    //first step: sort the palette based upon usage count
    for (index = 0; index < paletteLength; index++)
    {
        sortList[index].index = index;
        sortList[index].usageCount = colorUsageCount[index];
    }
    qsort(sortList, paletteLength, sizeof(qtablesortstruct), qReductionSortCB);
    //next, the first length entries in the dest palette are to be taken directly from the sorted palette
    for (index = 0; index < length; index++)
    {
        destPalette[base + index] = palette[sortList[index].index];//set new palette entry
        destTable[sortList[index].index] = (ubyte)(base + index);//set mapping table entry
    }
    //finally, the rest of the table must be the best matches
    //  we can find among the colors we have chosen.
    for (; index < paletteLength; index++)
    {
        destTable[sortList[index].index] = (ubyte)(base +
            colBestFitFindRGB(destPalette + base, palette[sortList[index].index], length));
    }
}

/*-----------------------------------------------------------------------------
    Name        : qRGBQuantizeQueue
    Description : Quantizes all stored-up images
    Inputs      : destPalette - where the palette is to go
    Outputs     : Uses the data stored in qQuantizeQueue to quantize a new palette.
    Return      : number of entries in new palette
    Note        : To save RAM in this rather costly venture, all the RGBSource
                    buffers from qQuantizeQueue will be freed as they are
                    copied to the master buffer.
----------------------------------------------------------------------------*/
sdword qRGBQuantizeQueue(color *destPalette)
{
    sdword index, pixel, size, teamCount;
    ubyte *pByte, *pByteDest;
    color *pColor, *pColorDest;
    uword *pWord;
    udword teamColorSize[Q_TeamColorSteps];
    color *RGBBuffer[Q_TeamColorSteps];
    ubyte *indexBuffer[Q_TeamColorSteps];
    ubyte *indexPointer[Q_TeamColorSteps];
    udword colorUsageCount[Q_TeamColorSteps][NUMBER_COLORS];
    udword paletteSize[Q_TeamColorSteps];
    udword newPaletteSize[Q_TeamColorSteps];
    udword totalColors, totalNewColors;
    color newPalette[Q_TeamColorSteps][NUMBER_COLORS];
    ubyte remappingTable[Q_TeamColorSteps][NUMBER_COLORS];

    //clear the team color size array
    for (index = 0; index < Q_TeamColorSteps; index++)
    {
        teamColorSize[index] = 0;
    }

    for (index = qImageSize = 0; index < qQuantizeQueueIndex; index++)
    {                                                    //compute total size of all images
        pixel = qQuantizeQueue[index].width * qQuantizeQueue[index].height;
        qImageSize += pixel;
        for (pByte = qQuantizeQueue[index].teamColor; pixel > 0; pixel--, pByte++)
        {                                                //count pixels in each team color level
            *pByte /= Q_TeamColorDivisor;                //convert to final precision
            teamColorSize[*pByte]++;                     //maintain count for each level
        }
    }
    //histograms can be big; let's allocate them dynamically so as not to waste
    //static storage
    qHistogram   = memAlloc(sizeof(real32) * 33 * 33 * 33, "qHistogram   ", 0);
    qHistoWeight = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoWeight ", 0);
    qHistoRed    = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoRed    ", 0);
    qHistoGreen  = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoGreen  ", 0);
    qHistoBlue   = memAlloc(sizeof(sdword) * 33 * 33 * 33, "qHistoBlue   ", 0);
    memset(colorUsageCount, 0, sizeof(udword) * NUMBER_COLORS * Q_TeamColorSteps);
    for (teamCount = 0; teamCount < Q_TeamColorSteps; teamCount++)
    {                                                       //for each level of team color
        //now that we know the size of all the images, we have to create the big buffer
        paletteSize[teamCount] = 0;
        if (teamColorSize[teamCount] == 0)
        {
            continue;
        }
        RGBBuffer[teamCount] = memAlloc(teamColorSize[teamCount] * sizeof(color), "BigRGBTeamBuffer", 0);
        qPalette = newPalette[teamCount];
        pColorDest = RGBBuffer[teamCount];
        for (index = 0; index < qQuantizeQueueIndex; index++)
        {                                                   //copy all images into one big happy buffer
            size = qQuantizeQueue[index].width * qQuantizeQueue[index].height;
            pByte = qQuantizeQueue[index].teamColor;
            pColor = qQuantizeQueue[index].RGBSource;
            for (; size > 0; size--, pByte++, pColor++)
            {                                               //for each pixel in each image at each team color level
                if (*pByte == (ubyte)teamCount)
                {                                           //if pixel is on this level
                    *pColorDest = *pColor;
                    pColorDest++;
                }
            }
//            if (teamCount == Q_TeamColorSteps - 1)
//            {                                               //if last level
//                memFree(qQuantizeQueue[index].RGBSource);   //free this buffer to save RAM space now when it is needed
//            }
        }
        dbgAssert(pColorDest - teamColorSize[teamCount] == RGBBuffer[teamCount]);
        qCalcIndices = FALSE;
/*
        if (qCalcIndices)
        {
            qIndexBuffer = memAlloc(teamColorSize[teamCount] * 2, "16-bit index image temp", 0);//alloc the index-mode buffer
        }
        else
        {
            qIndexBuffer = NULL;
        }
*/
        //because there is the assumption that the static data is all zeros,
        //we must now clear out all these dynamic arrays
        memset(qHistogram, 0, sizeof(real32) * 33 * 33 * 33);
        memset(qHistoWeight, 0, sizeof(sdword) * 33 * 33 * 33);
        memset(qHistoRed, 0, sizeof(sdword) * 33 * 33 * 33);
        memset(qHistoGreen, 0, sizeof(sdword) * 33 * 33 * 33);
        memset(qHistoBlue, 0, sizeof(sdword) * 33 * 33 * 33);

        qRGBBuffer = RGBBuffer[teamCount];                  //reference the current buffers
        qImageSize = teamColorSize[teamCount];
        qColorUsageCount = colorUsageCount[teamCount];
        quantizeMain(NUMBER_COLORS);
        //save the RGB buffer for later use
//!!!        memFree(qRGBBuffer);                                //free the current RGB buffer because it's done with
        paletteSize[teamCount] = qCLUTSize;                 //record histogram size

    }


    //at this point we have 4 palettes and information on how much each color is used.
    //we must perform some statistical analysis on these palettes and create a proper destination palette.
    //also, the final indexed images need be created at this point.

    //first, let's do a pass to compute number of colors per level
    //the algorithm here does not consider importance of colors based upon usage count
    for (totalColors = teamCount = 0; teamCount < Q_TeamColorSteps; teamCount++)
    {
        totalColors += paletteSize[teamCount];
    }
    //!!! this is an area where the color spaces could be optimized.  This
    //distribution of palette space is not very efficient and produces some
    //visual artifacts.
#if Q_PRINT_COLOR_ALLOCS
    printf("\nColorSpace(%d): ", Q_TeamColorSteps);
#endif
    for (totalNewColors = teamCount = 0; teamCount < Q_TeamColorSteps; teamCount++)
    {
        newPaletteSize[teamCount] = paletteSize[teamCount] * NUMBER_COLORS / totalColors;
        if (newPaletteSize[teamCount] > paletteSize[teamCount])
        {
            newPaletteSize[teamCount] = paletteSize[teamCount];
        }
        dbgAssert(newPaletteSize[teamCount] <= paletteSize[teamCount]);
        totalNewColors += newPaletteSize[teamCount];
#if Q_PRINT_COLOR_ALLOCS
        printf("(%d,%d,%d) ", teamColorSize[teamCount], paletteSize[teamCount], newPaletteSize[teamCount]);
#endif
//      if (teamColorSize[teamCount] == 0)
//      {
//          continue;
//      }
    }
    printf("\n");

    //@@@ instead of just reducing the palette, re-quantize each chunk into it's allocated space
    dbgAssert(totalNewColors <= NUMBER_COLORS);                      //verify we've not exceeded max palette size

    for (index = teamCount = 0; teamCount < Q_TeamColorSteps; teamCount++)
    {
        if (teamColorSize[teamCount] == 0)
        {
            continue;
        }
        qCalcIndices = TRUE;
        if (qCalcIndices)
        {
            qIndexBuffer = memAlloc(teamColorSize[teamCount] * 2, "16-bit index image temp", 0);//alloc the index-mode buffer
        }
        else
        {
            qIndexBuffer = NULL;
        }

        //clear out all these dynamic arrays
        memset(qHistogram, 0, sizeof(real32) * 33 * 33 * 33);
        memset(qHistoWeight, 0, sizeof(sdword) * 33 * 33 * 33);
        memset(qHistoRed, 0, sizeof(sdword) * 33 * 33 * 33);
        memset(qHistoGreen, 0, sizeof(sdword) * 33 * 33 * 33);
        memset(qHistoBlue, 0, sizeof(sdword) * 33 * 33 * 33);

        qPalette = newPalette[teamCount];
        qRGBBuffer = RGBBuffer[teamCount];                  //reference the current buffers
        qImageSize = teamColorSize[teamCount];
        qColorUsageCount = colorUsageCount[teamCount];
        quantizeMain(newPaletteSize[teamCount]);            //quantize to the new size
        //save the RGB buffer for later use
//!!!        memFree(qRGBBuffer);                                //free the current RGB buffer because it's done with
        dbgAssert(qCLUTSize == (sdword)newPaletteSize[teamCount]);

        //copy the computed palette into the common palette
        memcpy(&destPalette[index], qPalette, sizeof(color) * newPaletteSize[teamCount]);
        //create a palette reduction table compatable with later code
        for (pixel = 0; pixel < (sdword)newPaletteSize[teamCount]; pixel++)
        {
			dbgAssert(index + pixel < 256);
            remappingTable[teamCount][pixel] = (ubyte)(index + pixel);
        }
        index += newPaletteSize[teamCount];                 //update amount of palette used
        //to save memory, convert the 16-bit indexed buffers to 8-bit
        // this may not really be needed
        if (qCalcIndices)
        {
            pByte = indexPointer[teamCount] = indexBuffer[teamCount] =//allocate 8-bit buffer
                memAlloc(teamColorSize[teamCount], "8-bitIndexTeamBuffer", 0);
            pWord = qIndexBuffer;
            for (pixel = teamColorSize[teamCount]; pixel > 0; pixel--, pByte++, pWord++)
            {                                               //convert from 16- to 8-bit
                *pByte = (ubyte)*pWord;
            }
            memFree(qIndexBuffer);                          //free the no-longer-needed 16-bit index buffer
        }
    }
    dbgAssert(index <= NUMBER_COLORS);                      //verify we haven't used too many colors
    dbgAssert(index == (sdword)totalNewColors);                     //or too few
/*
    for (index = teamCount = 0; teamCount < Q_TeamColorSteps; teamCount++)
    {
        if (teamColorSize[teamCount] == 0)
        {
            continue;
        }
        qPaletteReductionTableCreate(remappingTable[teamCount], destPalette, index,
                                     newPaletteSize[teamCount], newPalette[teamCount],
                                     paletteSize[teamCount], colorUsageCount[teamCount]);
        index += newPaletteSize[teamCount];
    }
    dbgAssert(index <= NUMBER_COLORS);                      //verify we haven't used too many colors
*/
    //free the no-longer-needed histograms
    memFree(qHistoBlue  );
    memFree(qHistoGreen );
    memFree(qHistoRed   );
    memFree(qHistoWeight);
    memFree(qHistogram  );
    //now that we have palettes, let's create a bunch of index-mode images
    //based upon new palettes and mapping tables and stuff
    if (qCalcIndices)
    {
        for (index = 0; index < qQuantizeQueueIndex; index++)
        {                                                   //for each image in queue
            size = qQuantizeQueue[index].width * qQuantizeQueue[index].height;
            pByte = qQuantizeQueue[index].teamColor;
            pByteDest = qQuantizeQueue[index].indexDest;
            for (; size > 0; size--, pByte++, pByteDest++)
            {
                dbgAssert(*pByte < Q_TeamColorSteps);
                *pByteDest = remappingTable[*pByte][*indexPointer[*pByte]];
                indexPointer[*pByte]++;
            }
            memFree(qQuantizeQueue[index].teamColor);       //this has served it's purpose well; delete it.
        }
    }

    //one final pass to perform clean-up
    for (teamCount = 0; teamCount < Q_TeamColorSteps; teamCount++)
    {
        if (teamColorSize[teamCount] == 0)
        {
            continue;
        }
        memFree(RGBBuffer[teamCount]);                      //free the RGB buffers because they're done with
        memFree(indexBuffer[teamCount]);                    //free the index buffers
    }

    qQuantizeQueueIndex = 0;
    return(totalNewColors);
}

