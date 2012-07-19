//
// list.h
// Paco's lists
//

#ifndef _LIST_H
#define _LIST_H

#include <list>
#include "lif.h"
#include "crc.h"

using namespace std;

#define PACF_Selected	1		//texture has been selected for packing
#define PACF_Packed		2		//texture has been packed

#define PACF_Small			4	//small texture, mutually exclusive w/ PACF_Large
#define PACF_Large			8	//large texture
#define PACF_Strange	    16	//extreme aspect
#define PACF_Square			32	//square texture

#define PACF_Alpha      128     //RGBA texture, mutually exclusive w/ PACF_Paletted
#define PACF_Paletted   256     //COLOR_INDEX texture

#define PACF_Rotated    512     //texture has been rotated

typedef struct lifpage
{
    char name[8];
	udword flags;               //expect PACF_Alpha or PACF_Paletted
    sdword width, height;		//dimensions of the LiF page
    sdword xOffset, yOffset;    //strips begin here
    lifheader* header;
} lifpage;

typedef lifpage* plifpage;

typedef struct liflist_s
{
	udword flags;               //PACF_* flags, !LiF flags

	char filename[1024];
    crc32 crc;
	lifheader* header;

	lifpage* parent;
	sdword packedX, packedY;
} liflist_t;

typedef list<plifpage> list_lifpage;
typedef list<liflist_t> list_lif;
typedef list<crc32> list_crc32;

extern list_lifpage lifpagelist;
extern list_lif liflist;
extern list_lif nopacklist;
extern list_lif notpackedlist;
extern list_crc32 crc32list;

#endif
