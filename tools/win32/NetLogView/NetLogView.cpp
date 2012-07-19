
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include "types.h"


#if 0

typedef struct binnetlogPacket
{
    udword header;
    uword packetnum;
    uword randcheck;
    real32 univcheck;
    udword blobcheck;
    uword numShipsInChecksum;
    uword numBlobsInChecksum;
    udword univUpdateCounter;
} binnetlogPacket;

typedef struct binnetlogCheatInfo
{
    udword header;
    udword totalships;
    udword resourceunits;
    udword shiptotals;
    udword classtotals;
    udword hastechnology;
    udword listoftopicsnum;
} binnetlogCheatInfo;

typedef struct binnetlogBountyInfo
{
    udword header;
    ubyte bounties[8];
} binnetlogBountyInfo;

typedef struct binnetlogShipInfo
{
    udword header;
    uword shipID;
    ubyte playerIndex;
    ubyte shiprace;
    ubyte shiptype;
    sbyte shiporder;
    sbyte shipattributes;
    ubyte tacticstype;
    ubyte isDodging;
    ubyte DodgeDir;
    real32 health;
    real32 x,y,z;
    real32 vx,vy,vz;
    real32 fuel;
} binnetlogShipInfo;

// resource info if harvesting
typedef struct binnetlogShipResourceInfo
{
    udword header;
    uword resourceID;
    real32 volume;
    real32 x,y,z;
} binnetlogShipResourceInfo;

// dock info if docking
typedef struct binnetlogShipDockInfo
{
    udword header;
    uword busyness;
    uword numDockPoints;
    udword thisDockBusy;
} binnetlogShipDockInfo;

// mad info if mesh animations
typedef struct binnetlogShipMadInfo
{
    udword header;
    ubyte info[8];
} binnetlogShipMadInfo;

typedef struct binnetlogBulletInfo
{
    udword header;
    uword bullettype;
    uword bulletplayerowner;
    uword bulletowner;
    real32 x,y,z;
    real32 vx,vy,vz;
    real32 timelived;
    real32 totallifetime;
    real32 traveldist;
    real32 damage;
    real32 damageFull;
    real32 DFGFieldEntryTime;
    real32 BulletSpeed;
    real32 collBlobSortDist;
} binnetlogBulletInfo;

typedef struct binnetDerelictInfo
{
    udword header;
    uword derelictid;
    uword derelicttype;
    real32 health;
    real32 x,y,z;
    real32 vx,vy,vz;
} binnetDerelictInfo;

typedef struct binnetResourceInfo
{
    udword header;
    uword resourceid;
    uword resourcetype;
    sdword resourceValue;
    real32 health;
    real32 x,y,z;
    real32 vx,vy,vz;
} binnetResourceInfo;

typedef struct binnetBlobInfo
{
    udword header;
    sdword numSpaceObjs;
    real32 collBlobSortDist;
    real32 x,y,z,r;
} binnetBlobInfo;

typedef struct binnetCmdLayerInfo
{
    udword header;
    uword order;
    uword attributes;
    uword numShips;
    uword ShipID[1];
} binnetCmdLayerInfo;

typedef struct binnetCmdLayerInfoMax
{
    udword header;
    uword order;
    uword attributes;
    uword numShips;
    uword ShipID[500];
} binnetCmdLayerInfoMax;

#define sizeofbinnetCmdLayerInfo(x) ( sizeof(binnetCmdLayerInfo) + sizeof(uword)*((x)-1) )

typedef struct binnetselection
{
    udword header;
    uword numShips;
    uword ShipID[1];
} binnetselection;

typedef struct binnetselectionMax
{
    udword header;
    uword numShips;
    uword ShipID[500];
} binnetselectionMax;

#define sizeofbinnetselection(x) ( sizeof(binnetselection) + sizeof(uword)*((x)-1) )

typedef struct binnetanyselection
{
    udword header;
    uword numTargets;
    udword TargetID[1];
} binnetanyselection;

typedef struct binnetanyselectionMax
{
    udword header;
    uword numTargets;
    udword TargetID[500];
} binnetanyselectionMax;

#define sizeofbinnetanyselection(x) ( sizeof(binnetanyselection) + sizeof(udword)*((x)-1) )

#endif


#define makenetcheckHeader(x,y,z,w) ((x) | ((y) << 8) | ((z) << 16) | ((w) << 24))

#define print printf

#define tsbyte      1
#define tubyte      2
#define tsword      3
#define tuword      4
#define tsdword     5
#define tudword     6
#define tsqword     7
#define tuqword     8
#define treal32     9
#define treal64     10
#define tbool32     11
#define tbool8      12
#define tbool16     13
#define tpad8       14
#define tpad16      15
#define tpad32      16
#define tpad64      17

typedef struct typestringpair
{
    sdword type;
    char *strname;
} typestringpair;

typestringpair packinfo[] =
{
    { makenetcheckHeader('P','P','P','P'), "Pack" },
    { tuword, "packetnum" },
    { tuword, "randcheck" },
    { treal32, "univcheck" },
    { tudword, "cheatcheck" },
    { tuword, "numShipsInChecksum" },
    { tuword, "numBlobsInChecksum" },
    { tudword, "univUpdateCounter" },
    { -1, "" }
};

typestringpair cheatinfo[] =
{
	{ makenetcheckHeader('C','D','E','T'), "CDET" },
	{ tudword, "totalships" },
    { tudword, "resourceunits" },
    { tudword, "shiptotals" },
    { tudword, "classtotals" },
    { tudword, "hastechnology" },
    { tudword, "listoftopicsnum" },
	{ -1, "" }
};

typestringpair bountyinfo[] =
{
	{ makenetcheckHeader('B','O','U','N'), "BOUNTY" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ tubyte, "" },
	{ -1, "" }
};

typestringpair shipinfo[] =
{
    { makenetcheckHeader('S','S','S','S'), "Ship" },
    { tuword, "shipID" },
    { tubyte, "playerIndex" },
    { tubyte, "shiprace" },
    { tubyte, "shiptype" },
    { tsbyte, "shiporder" },
    { tsbyte, "shipattributes" },
    { tubyte, "tacticstype" },
    { tubyte, "isDodging" },
    { tubyte, "DodgeDir" },
    { tpad16, "pad"},
    { treal32, "health" },
    { treal32, "x" },
    { treal32, "y" },
    { treal32, "z" },
    { treal32, "vx" },
    { treal32, "vy" },
    { treal32, "vz" },
    { treal32, "fuel" },
    { -1, "" }
};


typestringpair shipresinfo[] =
{
    { makenetcheckHeader('S','R','S','R'), "SR" },
    { tuword, "resourceID" },
    { tpad16, "pad" },
    { treal32, "volume" },
    { treal32, "x" },
    { treal32, "y" },
    { treal32, "z" },
    { -1, "" }
};


typestringpair shipdockinfo[] =
{
    { makenetcheckHeader('S','D','S','D'), "SD" },
    { tuword, "busyness" },
    { tuword, "numDockPoints" },
    { tudword, "thisDockBusy" },
    { -1, "" }
};

// mad info if mesh animations
typestringpair shipmadinfo[] =
{
    { makenetcheckHeader('S','M','S','M'), "SM" },
    { tubyte, "info0" },
    { tubyte, "info1" },
    { tubyte, "info2" },
    { tubyte, "info3" },
    { tubyte, "info4" },
    { tubyte, "info5" },
    { tubyte, "info6" },
    { tubyte, "info7" },
    { -1, "" }
};

typestringpair bulletinfo[] =
{
    { makenetcheckHeader('B','B','B','B'), "BUL" },
    { tuword, "bullettype" },
    { tuword, "bulletplayerowner" },
    { tuword, "bulletowner" },
    { tpad16, "pad" },
    { treal32, "x" },
    { treal32, "y" },
    { treal32, "z" },
    { treal32, "vx" },
    { treal32, "vy" },
    { treal32, "vz" },
    { treal32, "timelived" },
    { treal32, "totallifetime" },
    { treal32, "traveldist" },
    { treal32, "damage" },
    { treal32, "damageFull" },
    { treal32, "DFGFieldEntryTime" },
    { tpad32, "BulletSpeed" },
    { treal32, "collBlobSortDist" },
    { -1, "" }
};

typestringpair derelictinfo[] =
{
    { makenetcheckHeader('D','D','D','D'), "DER" },
    { tuword, "derelictid" },
    { tuword, "derelicttype" },
    { treal32, "health" },
    { treal32, "x" },
    { treal32, "y" },
    { treal32, "z" },
    { treal32, "vx" },
    { treal32, "vy" },
    { treal32, "vz" },
    { -1, "" }
};

typestringpair resourceinfo[] =
{
    { makenetcheckHeader('R','R','R','R'), "RES" },
    { tuword, "resourceid" },
    { tuword, "resourcetype" },
    { tsdword, "resourceValue" },
    { treal32, "health" },
    { treal32, "x" },
    { treal32, "y" },
    { treal32, "z" },
    { treal32, "vx" },
    { treal32, "vy" },
    { treal32, "vz" },
    { -1, "" }
};

typestringpair blobinfo[] =
{
    { makenetcheckHeader('B','1','0','B'), "BLOB" },
    { tsdword, "numSpaceObjs" },
    { treal32, "collBlobSortDist" },
    { treal32, "x" },
    { treal32, "y" },
    { treal32, "z" },
    { treal32, "r" },
    { -1, "" }
};

typedef struct
{
    udword validheader;
    typestringpair *usetypestringpair;
} listofvalidheaders;

listofvalidheaders listValidHeaders[] =
{
    { makenetcheckHeader('P','P','P','P'),packinfo },
	{ makenetcheckHeader('C','D','E','T'),cheatinfo },
	{ makenetcheckHeader('B','O','U','N'),bountyinfo },
    { makenetcheckHeader('S','S','S','S'),shipinfo },
    { makenetcheckHeader('B','1','0','B'),blobinfo },
#if 0
    makenetcheckHeader('C','C','C','C'),
    makenetcheckHeader('F','O','R','M'),
    makenetcheckHeader('S','E','P','T'),
    makenetcheckHeader('S','E','P','A'),
    makenetcheckHeader('A','T','M','V'),
    makenetcheckHeader('M','O','V','E'),
    makenetcheckHeader('S','E','A','T'),
    makenetcheckHeader('D','O','C','K'),
    makenetcheckHeader('L','A','U','N'),
    makenetcheckHeader('B','U','L','D'),
    makenetcheckHeader('S','P','E','C'),
    makenetcheckHeader('M','I','L','P'),
    makenetcheckHeader('M','P','H','P'),
#endif
    { makenetcheckHeader('S','R','S','R'),shipresinfo },
    { makenetcheckHeader('S','D','S','D'),shipdockinfo },
    { makenetcheckHeader('S','M','S','M'),shipmadinfo },
    { makenetcheckHeader('B','B','B','B'),bulletinfo },
    { makenetcheckHeader('D','D','D','D'),derelictinfo },
    { makenetcheckHeader('R','R','R','R'),resourceinfo },
    { (udword)-1, (typestringpair *)NULL }
};

int verbose = 0;
FILE *fp;

bool ReadAndPrintNetLogInfo(listofvalidheaders *validheader)
{
    typestringpair *usetypestringpair = validheader->usetypestringpair;
    udword type;

    sbyte sb;
    sword sw;
    sdword sdw;
    sqword sqw;
    real32 r32;
    real64 r64;

    if ((udword)usetypestringpair->type != validheader->validheader)
    {
        _asm int 3
    }
    print("\n");
    print(usetypestringpair->strname);

    usetypestringpair++;
    while (usetypestringpair->type != -1)
    {
        type = usetypestringpair->type;

        if ((type >= tpad8) && (type <= tpad64))
        {
            ;
        }
        else
        {
			if ((usetypestringpair->strname != NULL) && (usetypestringpair->strname[0]))
			{
				print(" ");
				print(usetypestringpair->strname);
			}
        }

        switch (type)
        {
            case tubyte:
            case tsbyte:
            case tbool8:
                if (fread(&sb,sizeof(sb),1,fp) == 0) return FALSE;
                print(" %d",sb);
                break;

            case tuword:
            case tsword:
            case tbool16:
                if (fread(&sw,sizeof(sw),1,fp) == 0) return FALSE;
                print(" %d",sw);
                break;

            case tudword:
            case tsdword:
            case tbool32:
                if (fread(&sdw,sizeof(sdw),1,fp) == 0) return FALSE;
                print(" %ld",sdw);
                break;

            case tuqword:
            case tsqword:
                if (fread(&sqw,sizeof(sqw),1,fp) == 0) return FALSE;
                print(" %I64d",sqw);       // hopefully this will work
                break;

            case treal32:
                if (fread(&r32,sizeof(r32),1,fp) == 0) return FALSE;
                print(" %f",r32);
                break;

            case treal64:
                if (fread(&r64,sizeof(r64),1,fp) == 0) return FALSE;
                print(" %f",r64);
                break;

            case tpad8:
                if (fread(&sb,sizeof(sb),1,fp) == 0) return FALSE;
                break;
            case tpad16:
                if (fread(&sw,sizeof(sw),1,fp) == 0) return FALSE;
                break;
            case tpad32:
                if (fread(&sdw,sizeof(sdw),1,fp) == 0) return FALSE;
                break;
            case tpad64:
                if (fread(&sqw,sizeof(sqw),1,fp) == 0) return FALSE;
                break;

            default:
                print("ERROR - unknown type %d",type);
                return FALSE;
        }
        usetypestringpair++;
    }

    return TRUE;
}

listofvalidheaders *IsHeaderValid(udword header)
{
    listofvalidheaders *i = &listValidHeaders[0];

    while (i->validheader != -1)
    {
        if (i->validheader == header)
        {
            return i;
        }

        i++;
    }

    return (listofvalidheaders *)NULL;
}

listofvalidheaders *ReadUntilNextHeader(void)
{
    udword header;
    listofvalidheaders *found = FALSE;
    ubyte nextbyte;

    if (fread(&header,sizeof(header),1,fp) == 0) return (listofvalidheaders *)NULL;

    if ((found = IsHeaderValid(header)) != NULL)
    {
        return found;
    }

    // ok, first 4 bytes we found isn't header, now we have to search byte by byte until we found a header:

    for (;;)
    {
        if (fread(&nextbyte,sizeof(nextbyte),1,fp) == 0) return (listofvalidheaders *)NULL;

        header >>= 8;
        header |= (nextbyte << 24);

        if ((found = IsHeaderValid(header)) != NULL)
        {
            return found;
        }
    }

    return (listofvalidheaders *)NULL;
}

int main(int argc,char *argv[])
{
    char *filename;

    listofvalidheaders *foundheader;

    if (argc < 2)
    {
        print("Usage: NetLogView netlog.txt [verbose]");
        return 0;
    }

    filename = argv[1];

    verbose = 0;
    if (argc >= 3)
    {
        if (stricmp(argv[2],"verbose") == 0)
        {
            verbose = 1;
        }
    }

    fp = fopen(filename,"rb");
    if (fp == NULL)
    {
        print("Error opening file %s",filename);
        return 0;
    }

    for (;;)
    {
        foundheader = ReadUntilNextHeader();
        if (foundheader == NULL)
            break;

        if (ReadAndPrintNetLogInfo(foundheader) == FALSE)
            break;
    }

    fclose(fp);
    return 0;
}

