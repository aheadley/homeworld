/*=============================================================================
    Name    : Autodownloadmap.c
    Purpose : Code for autodownloading maps

    Created 99/03/29 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#include <direct.h>
#else
    #include <sys/stat.h>
    #include <dirent.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Types.h"
#include "Memory.h"
#include "Debug.h"
#include "File.h"
#include "CommandNetwork.h"
#include "Titan.h"
#include "HorseRace.h"
#include "StringsOnly.h"

#define NUM_FILES_OF_MAP_GROW       20

typedef struct FileOfMapInfo
{
    char filename[100];
    char filedirname[100];
    sdword fileSize;
    sdword fileCRC;
} FileOfMapInfo;

typedef struct AutoDownloadMapInfo
{
    char mapname[100];      // note, includes %d of players, e.g. Curtain%d
    char dirname[100];      // e.g., Multiplayer\\Curtain%d\\            directory
    char exactmapname[100]; // same as mapname but with actual number of players
    char exactdirname[100]; // same as dirname but with actual number of players
    sdword numPlayers;
    sdword minPlayers;      // min players map can support
    sdword maxPlayers;      // max players map can support
    bool needAutoDownload;
    sdword numFilesSent;
    sdword numFilesAutodownloaded;      // only has meaning if you are the one downloading
    sdword totalFilesToAutodownload;    // only has meaning if you are the one downloading
    sdword mapCRC[MAX_MULTIPLAYER_PLAYERS];
    bool8 playersNeedMap[MAX_MULTIPLAYER_PLAYERS];
    sdword numFilesOfMap;
    sdword numFilesOfMapAllocated;
    FileOfMapInfo *fileOfMapInfo;
} AutoDownloadMapInfo;

AutoDownloadMapInfo autodownloadmapInfo;


FilePacket *CreateFilePacket(char *dirname,char *filename,sdword filecontentssize);

/*=============================================================================
    Functions:
=============================================================================*/

void autodownloadmapStartup()
{
    memset(&autodownloadmapInfo,0,sizeof(autodownloadmapInfo));
}

void autodownloadmapShutdown()
{
    if (autodownloadmapInfo.fileOfMapInfo)
    {
        memFree(autodownloadmapInfo.fileOfMapInfo);
        autodownloadmapInfo.fileOfMapInfo = NULL;
    }

    memset(&autodownloadmapInfo,0,sizeof(autodownloadmapInfo));
}

void autodownloadmapReset()
{
    autodownloadmapShutdown();
}

void GetExactMapDirNames(sdword num)
{
    sprintf(autodownloadmapInfo.exactmapname,autodownloadmapInfo.mapname,num);
    sprintf(autodownloadmapInfo.exactdirname,autodownloadmapInfo.dirname,num);
}

bool autodownloadmapBuiltIn()
{
    char fullfile[200];

    GetExactMapDirNames(autodownloadmapInfo.numPlayers);

    strcpy(fullfile,autodownloadmapInfo.exactdirname);
    strcat(fullfile,autodownloadmapInfo.exactmapname);
    strcat(fullfile,".level");

    if (fileExistsInBigFile(fullfile))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void autodownloadmapGotMapName(char *mapname,sdword numPlayers,sdword minPlayers,sdword maxPlayers)
{
    strcpy(autodownloadmapInfo.mapname,mapname);

#ifdef _WIN32
    strcpy(autodownloadmapInfo.dirname,"Multiplayer\\");
    strcat(autodownloadmapInfo.dirname,autodownloadmapInfo.mapname);
    strcat(autodownloadmapInfo.dirname,"\\");
#else
    strcpy(autodownloadmapInfo.dirname,"Multiplayer/");
    strcat(autodownloadmapInfo.dirname,autodownloadmapInfo.mapname);
    strcat(autodownloadmapInfo.dirname,"/");
#endif

    autodownloadmapInfo.numPlayers = numPlayers;
    autodownloadmapInfo.minPlayers = minPlayers;
    autodownloadmapInfo.maxPlayers = maxPlayers;

    if ((!autodownloadmapBuiltIn()) && (sigsNumPlayers > 1))       // sigsNumPlayers is # of network players, numPlayers includes computer players
    {
        autodownloadmapInfo.needAutoDownload = TRUE;
    }
}

bool autodownloadmapRequired()
{
    return autodownloadmapInfo.needAutoDownload;
}

void AddFileOfMap(char *filedirname,char *filename,sdword fileSize)
{
    sdword index = autodownloadmapInfo.numFilesOfMap++;

    if (autodownloadmapInfo.numFilesOfMap > autodownloadmapInfo.numFilesOfMapAllocated)
    {
        if (autodownloadmapInfo.fileOfMapInfo == NULL)
        {
            autodownloadmapInfo.fileOfMapInfo = memAlloc(sizeof(FileOfMapInfo)*NUM_FILES_OF_MAP_GROW,"fileofmap",0);
        }
        else
        {
            autodownloadmapInfo.fileOfMapInfo = memRealloc(autodownloadmapInfo.fileOfMapInfo,sizeof(FileOfMapInfo)*(autodownloadmapInfo.numFilesOfMapAllocated + NUM_FILES_OF_MAP_GROW),"rfileofmap",0);
        }

        autodownloadmapInfo.numFilesOfMapAllocated += NUM_FILES_OF_MAP_GROW;
    }

    strcpy(autodownloadmapInfo.fileOfMapInfo[index].filename,filename);
    strcpy(autodownloadmapInfo.fileOfMapInfo[index].filedirname,filedirname);
    autodownloadmapInfo.fileOfMapInfo[index].fileSize = fileSize;
    autodownloadmapInfo.fileOfMapInfo[index].fileCRC = 0;
}

void autodownloadmapGetFilesOfMap(void)
{
#ifdef _WIN32
    struct _finddata_t find;
    sdword handle, startHandle;
#else
    DIR* dp;
    struct dirent* dir_entry;
    unsigned int str_len, j;
#endif
    char dir[200];
    char file[200];
    sdword fileSize;
    sdword i;

    autodownloadmapInfo.numFilesOfMap = 0;

    for (i=autodownloadmapInfo.minPlayers;i<=autodownloadmapInfo.maxPlayers;i++)
    {
        GetExactMapDirNames(i);

        strcpy(dir,filePathPrepend(autodownloadmapInfo.exactdirname,0));
#ifdef _WIN32
        strcat(dir,"\\*.*");

        startHandle = handle = _findfirst(dir, &find);

        while (handle != -1)
        {
            if (find.name[0] == '.')
            {
                goto next;
            }

            if (strstr(find.name,".mdb"))
            {
                goto next;      // don't send .mdb files
            }

            strcpy(file,autodownloadmapInfo.exactdirname);
            strcat(file,find.name);

            fileSize = fileSizeGet(file,FF_IgnoreBIG);

            if ((fileSize <= 0) || (fileSize > 64000))
            {
                goto next;      // file too big
            }

            AddFileOfMap(autodownloadmapInfo.exactdirname,find.name,fileSize);

    next:
            handle = _findnext(startHandle, &find);
        }
#else
        str_len = strlen(dir);
        for (j = 0; j < str_len; j++)
        {
            if (dir[j] == '\\')
                dir[j] = '/';
        }

        /* Open the directory. */
        dp = opendir(dir);
        if (!dp)
            continue;

        /* Cycle through each directory entry. */
        while ((dir_entry = readdir(dp)))
        {
            if (dir_entry->d_name[0] == '.')
                continue;

            if (strstr(dir_entry->d_name, ".mdb"))
                continue;

            strcpy(file, autodownloadmapInfo.exactdirname);
            strcat(file, dir_entry->d_name);

            fileSize = fileSizeGet(file, FF_IgnoreBIG);

            if (fileSize <= 0 || fileSize > 64000)
                continue;

            AddFileOfMap(autodownloadmapInfo.exactdirname, dir_entry->d_name,
                fileSize);
        }

        closedir(dp);
#endif
    }
}

void SendFile(char *dirname,char *filename,sdword fileSize)
{
    FilePacket *pkt;

    pkt = CreateFilePacket(dirname,filename,fileSize);
    // for now, just broadcast pkt
    titanDebug("autodownloadmap: Sending file packet of size %d\n",sizeofFilePacketGivenPacket(pkt));
    titanSendBroadcastMessage((ubyte *)pkt,sizeofFilePacketGivenPacket(pkt));
}

bool autodownloadmapSendAFile(void)
{
    sdword i = autodownloadmapInfo.numFilesSent;

    if (i >= autodownloadmapInfo.numFilesOfMap)
    {
        return TRUE;
    }

    SendFile(autodownloadmapInfo.fileOfMapInfo[i].filedirname,autodownloadmapInfo.fileOfMapInfo[i].filename,autodownloadmapInfo.fileOfMapInfo[i].fileSize);
    autodownloadmapInfo.numFilesSent++;

    if (autodownloadmapInfo.numFilesSent >= autodownloadmapInfo.numFilesOfMap)
    {
        return TRUE;
    }

    return FALSE;
}

void autodownloadmapSendAllFiles(void)
{
    sdword i;

    for (i=0;i<autodownloadmapInfo.numFilesOfMap;i++)
    {
        SendFile(autodownloadmapInfo.fileOfMapInfo[i].filedirname,autodownloadmapInfo.fileOfMapInfo[i].filename,autodownloadmapInfo.fileOfMapInfo[i].fileSize);
    }
}

bool autodownloadmapReceivedAllFiles()
{
    if (autodownloadmapInfo.numFilesAutodownloaded == 0)
    {
        return FALSE;
    }

    if (autodownloadmapInfo.numFilesAutodownloaded >= autodownloadmapInfo.totalFilesToAutodownload)
    {
        return TRUE;
    }

    return FALSE;
}

real32 autodownloadmapPercentReceivedFiles()
{
    if (autodownloadmapInfo.numFilesAutodownloaded == 0)
    {
        return 0.0f;
    }

    return (real32)autodownloadmapInfo.numFilesAutodownloaded / (real32)autodownloadmapInfo.totalFilesToAutodownload;
}

real32 autodownloadmapPercentSentFiles()
{
    if (autodownloadmapInfo.numFilesSent == 0)
    {
        return 0.0f;
    }

    return (real32)autodownloadmapInfo.numFilesSent / (real32)autodownloadmapInfo.numFilesOfMap;
}

FilePacket *CreateFilePacket(char *dirname,char *filename,sdword filecontentssize)
{
    char file[200];
    sdword packetsize;
    sdword filenamesize;
    FilePacket *packet;

    strcpy(file,dirname);
    strcat(file,filename);

    filenamesize = strlen(file) + 1;

    packetsize = sizeofFilePacket(filenamesize,filecontentssize);

    packet = memAlloc(packetsize,"filepkt",0);

    packet->packetheader.type = PACKETTYPE_FILE;
    packet->packetheader.from = sigsPlayerIndex;
    packet->packetheader.frame = filecontentssize;
    packet->packetheader.numberOfCommands = filenamesize;

    packet->totalNumFiles = autodownloadmapInfo.numFilesOfMap;
    strcpy(packet->filename,file);
    fileLoad(file, filecontentsAddress(packet,filenamesize), FF_IgnoreBIG);

    return packet;
}

void SaveFilePacketToFile(FilePacket *fpacket)
{
    // lets write file to filesystem, but don't use file.h since this is in another thread
    // and file.c, file.h is not reentrant (and we're not writing to a big file)

    FILE *outFile;
    char file[PATH_MAX];
    char *ch0, ch1;
    sdword lengthWrote;
    sdword filecontentssize = fpacket->packetheader.frame;
    sdword filenamesize = fpacket->packetheader.numberOfCommands;

    strcpy(file,filePrependPath);
    strcat(file,fpacket->filename);

    if (!fileMakeDestinationDirectory(file))
    {
        titanDebug("autodownloadmap: WARNING could not create destination "
                   "directory for %s\n",
                   file);
        return;
    }

    if ((outFile = fopen(file, "wb")) == NULL)           //open the file
    {
        titanDebug("autodownloadmap: WARNING could not open file %s for writing\n",file);
        return;
    }

    lengthWrote = fwrite(filecontentsAddress(fpacket,filenamesize),1,filecontentssize,outFile);
    if (lengthWrote != filecontentssize)
    {
        titanDebug("autodownloadmap: ERROR tried to write %d bytes only wrote %d bytes to file %s\n",filecontentssize,lengthWrote,file);
    }

    fclose(outFile);
}

void receivedFilePacketCB(ubyte *packet,udword sizeofPacket)
{
    titanDebug("autodownloadmap: Received file packet of size %d\n",sizeofFilePacketGivenPacket((FilePacket *)packet));

    if (sigsPlayerIndex == 0)
    {
        titanDebug("autodownloadmap: WARNING: player 0 received file");
        return;         // throw away packet
    }
#define fpacket ((FilePacket *)packet)

    // lets write file to filesystem, but don't use file.h since this is in another thread
    // and file.c, file.h is not reentrant (and we're not writing to a big file)

    {
        // let's create a directory if necessary
        char dirtomake[250];
        char *findlastslashptr;

        strcpy(dirtomake,filePrependPath);
        strcat(dirtomake,fpacket->filename);
        findlastslashptr = &dirtomake[strlen(dirtomake)];

        while (--findlastslashptr >= dirtomake)
        {
#ifdef _WIN32
            if (*findlastslashptr == '\\')
#else
            if (*findlastslashptr == '\\' || *findlastslashptr == '/')
#endif
            {
                *findlastslashptr = 0;
                break;
            }
        }

        dbgAssert(*findlastslashptr == 0);

#ifdef _WIN32
        _mkdir(dirtomake);      // try to make directory.  If it already exists, this will fail, that's ok
#else
        mkdir(dirtomake, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    }

    dbgAssert(sizeofPacket == sizeofFilePacketGivenPacket(fpacket));
    SaveFilePacketToFile(fpacket);

    autodownloadmapInfo.totalFilesToAutodownload = fpacket->totalNumFiles;
    autodownloadmapInfo.numFilesAutodownloaded++;
#undef fpacket
}

void autodownloadmapPrintStatusStart()
{
    if (sigsPlayerIndex == 0)
    {
        strcpy(horseracestatus.hrstatusstr[sigsPlayerIndex],strGetString(strCustomMapAutoupload));
    }
    else
    {
        strcpy(horseracestatus.hrstatusstr[sigsPlayerIndex],strGetString(strCustomMapAutodownload));
    }
}

void autodownloadmapPrintStatus()
{
#if 0
    if (sigsPlayerIndex != 0)
    {
        if (autodownloadmapInfo.numFilesAutodownloaded != 0)
        {
            sprintf(horseracestatus.hrstatusstr[sigsPlayerIndex],"Detected User Map - Autodownloaded %d/%d files",autodownloadmapInfo.numFilesAutodownloaded,autodownloadmapInfo.totalFilesToAutodownload);
        }
    }
#endif
}

