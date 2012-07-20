/*=============================================================================
    Name    : Netcheck.h
    Purpose : Definitions for Netcheck.c

    Created 8/3/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___NETCHECK_H
#define ___NETCHECK_H

#include "Switches.h"
#include "Types.h"
#include "File.h"
#include "CommandNetwork.h"

#define PKTS_EXTENSION  ".pkts"

#define BINNETLOG       1

void netcheckReset(void);
void netcheckInit(void);
void netcheckClose(void);
void netcheckShow(HWPacketHeader *packet);
#if SYNC_CHECK
void netcheckFillInChecksum(HWPacketHeader *packet);
void netcheckCheckChecksum(NetSyncChecksums *checksum);
#endif
//void netcheckShow(HWPacketHeader *packet);
//void netcheckFillInChecksum(HWPacketHeader *packet);
//void netcheckCheckChecksum(NetSyncChecksums *checksum);

/*=============================================================================
    Public Variables:
=============================================================================*/

#if SYNC_CHECK
extern udword randSyncErr;
extern udword univSyncErr;
extern udword blobSyncErr;
extern udword pktSyncErr;

extern udword randSyncErrFrame;
extern udword univSyncErrFrame;
extern udword blobSyncErrFrame;
#endif

extern FILE *netlogfile;

void recPackRecordInit(void);
void recPackRecordPacket(ubyte *packet,udword sizeofPacket);
void recPackRecordPacketFilename(ubyte *packet,udword sizeofPacket,char *filename);
ubyte *recPackPlayGetNextPacket(udword *sizeofPacket);
void recPackPlayInit(void);
void recPackPlayClose(void);
void recPackPlayInGameInit(void);
void recPackInGameStartCB(char *filename);
void recPackInGameStartCBSafeToStart();
void recPackInGameStopCB(void);

extern char OrigRecordPacketFileName[];

#ifdef GOD_LIKE_SYNC_CHECKING
void syncDebugDump(char *filename,sdword counter,bool save);
void industrialStrengthSyncDebugging(sdword FrameNumber);
void netcheckIndustrialChecksum();
void netReceivedSyncFromNonCaptain(void *checksums,udword frame,udword playerIndex);
void godSyncInit();
void godSyncShutDown();

#endif

#if BINNETLOG

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

#define makenetcheckHeader(x,y,z,w) ((x) | ((y) << 8) | ((z) << 16) | ((w) << 24))

#endif

#endif

