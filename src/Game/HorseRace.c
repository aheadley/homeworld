/*=============================================================================
    Name    : HorseRace.c
    Purpose : Logic for the horse race and loading chat screens.

    Created 6/16/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "Types.h"
#include "Globals.h"
#include "glinc.h"
#include "Types.h"
#include "Debug.h"
#include "prim2d.h"
#include "Universe.h"
#include "render.h"
#include "CommandNetwork.h"
#include "HorseRace.h"
#include "Region.h"
#include "UIControls.h"
#include "FEFlow.h"
#include "font.h"
#include "FontReg.h"
#include "prim2d.h"
#include "utility.h"
#include "Task.h"
#include "Demo.h"
#include "mouse.h"
#include "LinkedList.h"
#include "Chatting.h"
#include "glcaps.h"
#include "NIS.h"
#include "MultiplayerGame.h"
#include "ETG.h"
#include "File.h"
#include "FEReg.h"
#include "interfce.h"
#include "Teams.h"
#include "TimeoutTimer.h"
#include "Strings.h"
#include "SinglePlayer.h"
#include "AutoDownloadMap.h"
#include "glcompat.h"
#include "texreg.h"
#include "Titan.h"
#include "devstats.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#define ShouldHaveMousePtr (FALSE)

extern Uint32 utyTimerLast;
extern udword gDevcaps2;

/*=============================================================================
    defines:
=============================================================================*/

#define HR_PlayerNameFont   "Arial_12.hff"
#define MAX_CHAT_TEXT       64
#define NUM_CHAT_LINES      10

real32 HorseRacePlayerDropoutTime = 10.0f;     // tweakable
color HorseRaceDropoutColor = colRGB(75,75,75);

/*=============================================================================
    data:
=============================================================================*/

/*extern HDC hGLDeviceContext;*/

static rectangle hrSinglePlayerPos;
static color hrSinglePlayerColor;

static sdword JustInit;
static sdword localbar;

// Pixels and info about the background image chosen
static bool hrBackgroundInitFrame = 0;
static udword *hrBackgroundImage = NULL;
static long hrBackgroundDirty = 0;
bool hrBackgroundReinit = FALSE;
static long hrBackXSize, hrBackYSize;

static regionhandle hrDecRegion;

HorseStatus horseracestatus;

TTimer hrPlayerDropoutTimers[MAX_MULTIPLAYER_PLAYERS];

fibfileheader *hrScreensHandle = NULL;

fonthandle      playernamefont=0;
color           hrBackBarColor=HR_BackBarColor;
color           hrChatTextColor=HR_ChatTextColor;

bool            PlayersAlreadyDrawnDropped[MAX_MULTIPLAYER_PLAYERS];

textentryhandle ChatTextEntryBox = NULL;

regionhandle   hrBaseRegion = NULL;
regionhandle   hrProgressRegion = NULL;
regionhandle   hrChatBoxRegion = NULL;

regionhandle   hrAbortLoadConfirm = NULL;

region horseCrapRegion =
{
    {-1,-1,-1,-1},          //rectangle
    NULL,                   //draw function
    NULL,                   //process function
    NULL, NULL,             //parent, child
    NULL, NULL,             //previous, next
    0, 0,                   //flags, status,
    0,                      //nKeys
    {0},                    //keys
    0,
#if REG_ERROR_CHECKING      //userID
    REG_ValidationKey,       //validation key
#endif
};

bool hrRunning=FALSE;

ChatPacket chathistory[NUM_CHAT_LINES];

udword      chatline=0;

void horseRaceRender(void);
void hrDrawPlayersProgress(featom *atom, regionhandle region);
void hrDrawChatBox(featom *atom, regionhandle region);
void hrChatTextEntry(char *name, featom *atom);
//void hrAbortNetworkGame(char *name, featom *atom);
//void hrAbortNonNetworkGame(char *name, featom *atom);

void hrAbortLoadingYes(char *name, featom *atom);
void hrAbortLoadingNo(char *name, featom *atom);

real32 horseRaceGetPacketPercent(real32 barPercent);

//HR_ChatTextEntry
//extern nisheader *utyTeaserHeader;

/*=============================================================================
    Functions
=============================================================================*/
fedrawcallback hrDrawCallback[] =
{
    {hrDrawPlayersProgress,     "HR_DrawPlayersProgress"    },
    {hrDrawChatBox,             "HR_DrawChatBox"            },
    {NULL,          NULL}
};

fecallback hrCallBack[] =
{
    {hrChatTextEntry,           "HR_ChatTextEntry"          },
    //{hrAbortNetworkGame,        "HR_AbortNetworkGame"       },
    //{hrAbortNonNetworkGame,     "HR_AbortNonNetworkGame"    },
    {hrAbortLoadingYes,         "HR_AbortLoadingYes"        },
    {hrAbortLoadingNo,          "HR_AbortLoadingNo"         },
    {NULL,          NULL}
};

void hrDirtyProgressBar()
{
    if (hrRunning)
    {
        if (hrProgressRegion != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(hrProgressRegion);
#endif
            hrProgressRegion->status |= RSF_DrawThisFrame;
        }
    }
}

void hrDirtyChatBox()
{
    if (hrRunning)
    {
        if (hrChatBoxRegion != NULL)
        {
#ifdef DEBUG_STOMP
            regVerify(hrChatBoxRegion);
#endif
            hrChatBoxRegion->status |= RSF_DrawThisFrame;
        }
    }
}

void hrBarDraw(rectangle *rect, color back, color fore, real32 percent)
{
    rectangle temp;
//  primRectSolid2(rect, back);

    if (percent > 1.0)
    {
        percent = 1.0;
    }

    temp.x0 = rect->x0;
    temp.y0 = rect->y0;
    temp.x1 = rect->x0 + (sdword)((rect->x1-rect->x0)*percent);
    temp.y1 = rect->y1;

    primRectSolid2(&temp, fore);
}

void hrDrawPlayersProgress(featom *atom, regionhandle region)
{
    sdword     index;
    rectangle pos;
    rectangle outline;
    real32 percent;
    fonthandle currentfont;
    bool droppedOut;

    hrProgressRegion = region;

    pos = region->rect;

//  primRectSolid2(&pos, colRGBA(0, 0, 0, 64));

    pos.y0+=fontHeight(" ");
    pos.y1=pos.y0+8;

    currentfont = fontMakeCurrent(playernamefont);

    if (multiPlayerGame)
    {
        dbgAssert(sigsNumPlayers == tpGameCreated.numPlayers);
        for (index=0;index<sigsNumPlayers;index++)
        {
            droppedOut = playerHasDroppedOutOrQuit(index);

            outline = pos;
            outline.x0 -= 5;
            outline.x1 += 10;
            outline.y0 -= 3;
            outline.y1 = outline.y0 + fontHeight(" ")*2 - 2;

            if ((hrBackgroundDirty) || (!PlayersAlreadyDrawnDropped[index]))
            {
                PlayersAlreadyDrawnDropped[index] = droppedOut;

#if 0
                if (hrBackgroundDirty)
                {
                    primRectTranslucent2(&outline, colRGBA(0,0,0,64));
                }
#else
                primRectSolid2(&outline, colBlack);
#endif

                if (droppedOut)
                {
                    fontPrintf(pos.x0,pos.y0,colBlack,"%s",tpGameCreated.playerInfo[index].PersonalName);
                    fontPrintf(pos.x0,pos.y0,HorseRaceDropoutColor,"%s",
                               (playersReadyToGo[index] == PLAYER_QUIT) ? strGetString(strQuit) : strGetString(strDroppedOut));
                }
                else
                {
                    fontPrintf(pos.x0,pos.y0,tpGameCreated.playerInfo[index].baseColor,"%s",tpGameCreated.playerInfo[index].PersonalName);

                    if (horseracestatus.hrstatusstr[index][0])
                    {
                        fontPrintf(pos.x0+150,pos.y0,tpGameCreated.playerInfo[index].baseColor,"%s",horseracestatus.hrstatusstr[index]);
                    }
                }
            }

            primRectOutline2(&outline, 1, (droppedOut) ? HorseRaceDropoutColor : tpGameCreated.playerInfo[index].stripeColor);

            pos.y0+=fontHeight(" ");
            pos.y1=pos.y0+4;

            percent = horseracestatus.percent[index];

            hrBarDraw(&pos,hrBackBarColor,(droppedOut) ? HorseRaceDropoutColor : tpGameCreated.playerInfo[index].baseColor,percent);

            pos.y0+=fontHeight(" ");
        }
    }
    else if (singlePlayerGame)
    {
        pos = hrSinglePlayerPos;

        if (pos.x0 != 0)
        {
            percent = horseracestatus.percent[0];

            //dbgMessagef("\npercent %f",percent);

            hrBarDraw(&pos, colBlack, hrSinglePlayerColor/*teColorSchemes[0].textureColor.base*/, percent);
        }
    }
    else
    {
        if (hrBackgroundDirty)
        {
            outline = pos;
            outline.x0 -= 5;
            outline.x1 += 10;
            outline.y0 -= 3;
            outline.y1 = outline.y0 + fontHeight(" ")*2 - 2;

            primRectTranslucent2(&outline, colRGBA(0,0,0,64));
            primRectOutline2(&outline, 1, teColorSchemes[0].textureColor.detail);

            fontPrintf(pos.x0,pos.y0,teColorSchemes[0].textureColor.base,"%s",playerNames[0]);
        }

        pos.y0+=fontHeight(" ");
        pos.y1=pos.y0+4;

        percent = horseracestatus.percent[0];

        hrBarDraw(&pos,hrBackBarColor,teColorSchemes[0].textureColor.base,percent);
    }
    fontMakeCurrent(currentfont);
}

void hrDrawChatBox(featom *atom, regionhandle region)
{
    fonthandle currentfont;
    sdword     x,y,i;
    char       name[128];

    hrChatBoxRegion = region;

    currentfont = fontMakeCurrent(playernamefont);

    primRectSolid2(&region->rect,colRGB(0,0,0));
    feStaticRectangleDraw(region);

    x = region->rect.x0+10;
    y = region->rect.y0;

    for (i=0;i<NUM_CHAT_LINES;i++)
    {
        if (chathistory[i].message[0]!=0)
        {
            x = region->rect.x0;
            sprintf(name,"%s >",tpGameCreated.playerInfo[chathistory[i].packetheader.frame].PersonalName);
            fontPrintf(x,y,tpGameCreated.playerInfo[chathistory[i].packetheader.frame].baseColor,"%s",name);
            x+=fontWidth(name)+10;
            //fontShadowSet(FS_E | FS_SE | FS_S);
            fontPrintf(x,y,hrChatTextColor,"%s",chathistory[i].message);
            //fontShadowSet(FS_NONE);
            y+= fontHeight(" ");
        }
    }

    fontMakeCurrent(currentfont);
}

void hrChooseSinglePlayerBitmap(char* pFilenameBuffer)
{
    char fname[128], line[64];
    filehandle handle;
    sdword x, y, width, height;
    sdword red, green, blue;

    memset(&hrSinglePlayerPos, 0, sizeof(hrSinglePlayerPos));

    //image itself
#if defined(OEM)
    if (singlePlayerGameInfo.currentMission == 5)
    {
#ifdef _WIN32
        sprintf(fname, "SinglePlayer\\mission05_OEM\\loading.jpg");
#else
        sprintf(fname, "SinglePlayer/mission05_OEM/loading.jpg");
#endif
    }
    else
#endif
#ifdef _WIN32
    sprintf(fname, "SinglePlayer\\mission%02d\\loading.jpg", singlePlayerGameInfo.currentMission);
#else
    sprintf(fname, "SinglePlayer/mission%02d/loading.jpg", singlePlayerGameInfo.currentMission);
#endif
    if (!fileExists(fname, 0))
    {
        pFilenameBuffer[0] = '\0';
        return;
    }

    strcpy(pFilenameBuffer, fname);

    //image script
    x = 42;
    y = 132 + ((RGLtype == SWtype) ? 1 : 0);
    width = 152;
    height = 3;
    x = feResRepositionX(x);
    y = feResRepositionY(y);
    hrSinglePlayerPos.x0 = x;
    hrSinglePlayerPos.y0 = y;
    hrSinglePlayerPos.x1 = x + width;
    hrSinglePlayerPos.y1 = y + height;

#ifdef _WIN32
    sprintf(fname, "SinglePlayer\\mission%02d\\loading.script", singlePlayerGameInfo.currentMission);
#else
    sprintf(fname, "SinglePlayer/mission%02d/loading.script", singlePlayerGameInfo.currentMission);
#endif
    if (!fileExists(fname, 0))
    {
        hrSinglePlayerColor = colRGB(255,63,63);
        return;
    }
    handle = fileOpen(fname, FF_TextMode);
    fileLineRead(handle, line, 63);
    sscanf(line, "%d %d %d", &red, &green, &blue);
    fileClose(handle);

    hrSinglePlayerColor = colRGB(red, green, blue);
}

void hrChooseRandomBitmap(char *pFilenameBuffer)
{
#ifdef _WIN32
struct _finddata_t  FindData;
    long hFind;
#else
    DIR *dp;
    struct dirent* dir_entry;
    FILE* fp;
#endif
    filehandle handle;
    long FileCount, BigFileCount, WhichFile, Result;
    char BigName[PATH_MAX], CurDir[PATH_MAX], NewDir[PATH_MAX];

    FileCount = BigFileCount = 0;

    /*GetCurrentDirectory(511, CurDir);*/
    getcwd(CurDir, PATH_MAX);

    // First, find screen shots listed in the BigFile
#ifdef _WIN32
    handle = fileOpen("ScreenShots\\ShotList.script", FF_ReturnNULLOnFail | FF_TextMode);
#else
    handle = fileOpen("ScreenShots/ShotList.script", FF_ReturnNULLOnFail | FF_TextMode);
#endif
    if(handle)
    {
        do {
            Result = fileLineRead(handle, BigName, 512);

            if(Result != FR_EndOfFile && Result > 0)    // Found one!
                BigFileCount++;

        } while(Result != FR_EndOfFile);

        fileClose(handle);
    }

    // Tell the file count how many pics were in the BigFile
    FileCount = BigFileCount;

    NewDir[0] = 0;
    strcpy(NewDir, filePathPrepend("ScreenShots", FF_UserSettingsPath));


    // Switch to the screenshots directory and count the ones in there
    /*SetCurrentDirectory(NewDir);*/
    chdir(NewDir);
#ifdef _WIN32
    hFind = _findfirst("*.jpg", &FindData);
    if(hFind != -1)
    {
        do {
            if( ((FindData.attrib & _A_SUBDIR) == 0) &&
                ((FindData.attrib & _A_HIDDEN) == 0) )
                FileCount++;
        } while (_findnext(hFind, &FindData) == 0);
        _findclose(hFind);
    }
#else
    dp = opendir(".");

    if (dp)
    {
        unsigned int dir_str_len;

        while ((dir_entry = readdir(dp)))
        {
            if (dir_entry->d_name[0] == '.')
                continue;
            dir_str_len = strlen(dir_entry->d_name);
            if (dir_str_len < 4 ||
                strcasecmp(dir_entry->d_name + dir_str_len - 4, ".jpg"))
                continue;

            /* See if the current process can actually open the file (simple
               check for read permissions and if it's a directory). */
            if (!(fp = fopen(dir_entry->d_name, "rb")))
                continue;
            fclose(fp);

            FileCount++;
        }

        closedir(dp);
    }
#endif

    // Did we find any at all?
    if(FileCount == 0)
    {
        pFilenameBuffer[0] = 0;
        /*SetCurrentDirectory(CurDir);*/
        chdir(CurDir);
        return;
    }

    WhichFile = (utyTimerLast % 32777) % FileCount;

    if(WhichFile < BigFileCount)
    {
        // The file we want is in the bigfile script
#ifdef _WIN32
        handle = fileOpen("ScreenShots\\ShotList.script", FF_ReturnNULLOnFail | FF_TextMode);
#else
        handle = fileOpen("ScreenShots/ShotList.script", FF_ReturnNULLOnFail | FF_TextMode);
#endif
        if(handle)
        {
            do {
                Result = fileLineRead(handle, BigName, 512);

                if(Result != FR_EndOfFile && Result > 0)    // Found one!
                    WhichFile--;

            } while( (WhichFile >= 0) && (Result != FR_EndOfFile));
#ifdef _WIN32
            strcpy(pFilenameBuffer, "ScreenShots\\");
#else
            strcpy(pFilenameBuffer, "ScreenShots/");
#endif
            strcat(pFilenameBuffer, BigName);
            /*SetCurrentDirectory(CurDir);*/
            chdir(CurDir);
            return;
        }
    }
    else
    {
        // The file we want is in the screenshots directory, so
        // remove BigFileCount from the file index we're looking for
        WhichFile -= BigFileCount;
        FileCount = 0;
#ifdef _WIN32
        hFind = _findfirst("*.jpg", &FindData);
        if(hFind != -1)
        {
            do {
                if( ((FindData.attrib & _A_SUBDIR) == 0) &&
                    ((FindData.attrib & _A_HIDDEN) == 0) )
                {
                    if(FileCount == WhichFile)
                    {
                        _findclose(hFind);
                        SetCurrentDirectory(CurDir);
                        strcpy(pFilenameBuffer, "ScreenShots\\");
                        strcat(pFilenameBuffer, FindData.name);
                        return;
                    }
                    else
                        FileCount++;
                }
            } while (_findnext(hFind, &FindData) == 0);
        }
#else
        dp = opendir(".");

        if (dp)
        {
            unsigned int dir_str_len;

            while ((dir_entry = readdir(dp)))
            {
                if (dir_entry->d_name[0] == '.')
                    continue;
                dir_str_len = strlen(dir_entry->d_name);
                if (dir_str_len < 4 ||
                    strcasecmp(dir_entry->d_name + dir_str_len - 4, ".jpg"))
                    continue;
                if (!(fp = fopen(dir_entry->d_name, "rb")))
                    continue;
                fclose(fp);

                if (FileCount != WhichFile)
                {
                    FileCount++;
                    continue;
    }

                strcpy(pFilenameBuffer, "ScreenShots/");
                strcat(pFilenameBuffer, dir_entry->d_name);
                break;
            }

            closedir(dp);
        }
#endif
    }
    /*SetCurrentDirectory(CurDir);*/
    chdir(CurDir);
}

typedef struct
{
    unsigned char v[2][2];
} ColorQuad;


//
// Takes X & Y values normalized from 0 to 1
// returns a bilinear interpolated pixel value
//
unsigned char hrBilinear(ColorQuad *pQuad, double X, double Y)
{
float Top, Bot, Final;
long FinalLong;

    Top = (float)pQuad->v[0][0] + ((float)pQuad->v[1][0] - (float)pQuad->v[0][0]) * X;
    Bot = (float)pQuad->v[0][1] + ((float)pQuad->v[1][1] - (float)pQuad->v[0][1]) * X;

    Final = (Top + (Bot - Top) * Y) + 0.5f;

    FinalLong = (long)Final;

    // Clamp it to 0 - 255
    if(FinalLong > 255) FinalLong = (FinalLong >> 31) & 0xff;
    return (unsigned char)FinalLong;
}

unsigned long hrGetInterpPixel(unsigned char *pSrcImg, long XSize, float Xf, float Yf)
{
unsigned long Result, x, y, xx, yy;
unsigned char *pCol;
double xfrac, yfrac, xint, yint;
ColorQuad r, g, b;

    x = (unsigned long)Xf;
    y = (unsigned long)Yf;

    xfrac = modf(Xf, &xint);
    yfrac = modf(Yf, &yint);

    for(yy=0; yy<2; yy++)
    {
        for(xx=0; xx<2; xx++)
        {
            pCol = &pSrcImg[ (((y+yy) * XSize) + x+xx) * 3 ];
            r.v[xx][yy] = pCol[0];
            g.v[xx][yy] = pCol[1];
            b.v[xx][yy] = pCol[2];
        }
    }

    Result = (unsigned long)hrBilinear(&r, xfrac, yfrac);
    Result |= (unsigned long)hrBilinear(&g, xfrac, yfrac) << 8;
    Result |= (unsigned long)hrBilinear(&b, xfrac, yfrac) << 16;
    Result |= 0xff000000;

    return Result;
}


long hrShipsToLoadForRace(ShipRace shiprace)
{
long ShipsToLoad = 0;
ShipType shiptype;
ShipType firstshiptype;
ShipType lastshiptype;
ShipStaticInfo *shipstaticinfo;

    firstshiptype = FirstShipTypeOfRace[shiprace];
    lastshiptype = LastShipTypeOfRace[shiprace];
    for (shiptype=firstshiptype;shiptype<=lastshiptype;shiptype++)
    {
        shipstaticinfo = GetShipStaticInfo(shiptype,shiprace);

        if( bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoNeeded) && !bitTest(shipstaticinfo->staticheader.infoFlags, IF_InfoLoaded) )
            ShipsToLoad++;
    }

    return ShipsToLoad;
}


long hrIsBackgroundWorthLoading(void)
{
long            ToLoad = 0;
ShipRace        shiprace;
AsteroidType    asteroidtype;
DustCloudType   dustcloudtype;
GasCloudType    gascloudtype;
NebulaType      nebulatype;
DerelictType    derelicttype;

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
        ToLoad += hrShipsToLoadForRace(shiprace);

    for (asteroidtype=0;asteroidtype<NUM_ASTEROIDTYPES;asteroidtype++)
    {
        if(bitTest(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(asteroidStaticInfos[asteroidtype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (dustcloudtype = 0; dustcloudtype < NUM_DUSTCLOUDTYPES; dustcloudtype++)
    {
        if(bitTest(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(dustcloudStaticInfos[dustcloudtype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (gascloudtype = 0; gascloudtype < NUM_GASCLOUDTYPES; gascloudtype++)
    {
        if(bitTest(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(gascloudStaticInfos[gascloudtype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (nebulatype = 0; nebulatype < NUM_NEBULATYPES; nebulatype++)
    {
        if(bitTest(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(nebulaStaticInfos[nebulatype].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    for (derelicttype=0;derelicttype<NUM_DERELICTTYPES;derelicttype++)
    {
        if(bitTest(derelictStaticInfos[derelicttype].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(derelictStaticInfos[derelicttype].staticheader.infoFlags, IF_InfoNeeded))
            ToLoad++;
    }

    for (shiprace=0;shiprace<NUM_RACES;shiprace++)
    {
        if(bitTest(missileStaticInfos[shiprace].staticheader.infoFlags, IF_InfoNeeded) && !bitTest(missileStaticInfos[shiprace].staticheader.infoFlags, IF_InfoLoaded))
            ToLoad++;
    }

    return(ToLoad > 5);
}

static bool hrDrawPixelsSupported(void)
{
    extern bool mainNoDrawPixels;

    if (mainNoDrawPixels)
    {
        //commandline option
        return FALSE;
    }
    if (mainSafeGL && (RGLtype == GLtype))
    {
        //running in "safe mode"
        return FALSE;
    }
    if ((RGLtype == GLtype) &&
        bitTest(gDevcaps2, DEVSTAT2_NO_DRAWPIXELS))
    {
        //bit in devcaps2
        return FALSE;
    }
    else if (strcasecmp(GLC_VENDOR, "ati") == 0 &&
             strstr(GLC_RENDERER, "128") != NULL)
    {
        //ATI Rage 128
        return FALSE;
    }
    else if (strstr(GLC_VENDOR, "Matrox") != NULL ||
             strstr(GLC_VENDOR, "atrox ") != NULL)
    {
        //Matrox cards
        return FALSE;
    }
    else
    {
        //assume support
        return TRUE;
    }
}

void hrInitBackground(void)
{
char hrImageName[PATH_MAX];
filehandle handle;
long XSize, YSize, i;
unsigned long *pDest;
unsigned char *pTempImage, *pTempLine, *pRGB;

float SrcX, SrcY, Scale;
float IncX, IncY;
long DestX, DestY;
long DestXSize, DestYSize, Size, Top, Bot;
JPEGDATA    jp;
char CurDir[PATH_MAX], NewDir[PATH_MAX];

    /*GetCurrentDirectory(511, CurDir);*/
    getcwd(CurDir, PATH_MAX);

    hrImageName[0] = 0;
    if (singlePlayerGame)
    {
        hrChooseSinglePlayerBitmap(hrImageName);
    }
    else
    {
//        if(hrIsBackgroundWorthLoading())
            hrChooseRandomBitmap( hrImageName );
    }

    /*GetCurrentDirectory(511, NewDir);*/
    getcwd(NewDir, PATH_MAX);

    dbgAssert(strcasecmp(CurDir,NewDir) == 0);

    // Load the bitmap image
    handle = fileOpen(hrImageName, FF_ReturnNULLOnFail);
    if(handle)
    {
        memset(&jp, 0, sizeof(jp));
        jp.input_file = handle;
        JpegInfo(&jp);

        fileSeek(handle, 0, SEEK_SET);

        XSize = jp.width;
        YSize = jp.height;

        pTempImage = (unsigned char *)memAllocAttempt((XSize+1) * (YSize+1) * 3, "BackgroundTemp", NonVolatile);
        if(pTempImage == NULL)
            return;

        jp.ptr = pTempImage;
        JpegRead(&jp);

        fileClose(handle);

        Size = XSize*3;
        pTempLine = (unsigned char *)malloc(Size);
        for(i=0; i<(YSize/2); i++)
        {
            Top = i;
            Bot = (YSize-1)-i;

            memcpy(pTempLine, pTempImage + (Size * Top), Size);
            memcpy(pTempImage + (Size * Top), pTempImage + (Size * Bot), Size);
            memcpy(pTempImage + (Size * Bot), pTempLine, Size);
        }
        free(pTempLine);

        // Replicate the last line to appease the filter algorithm
        memcpy(&pTempImage[YSize * XSize * 3], &pTempImage[(YSize-1) * XSize * 3], XSize*3);

        if (singlePlayerGame)
        {
            //fixed width for singleplayer game images
            DestXSize = 640;
            DestYSize = 480;
            hrBackgroundImage = (udword*)malloc(DestXSize * DestYSize * 4);
        }
        else
        {
            if (!hrDrawPixelsSupported())
            {
                //no DrawPixels support, must use glcompat 640x480 quilting
                DestXSize = 640;
                DestYSize = 480;
                hrBackgroundImage = (udword*)malloc(DestXSize * DestYSize * 4);
            }
            else
            {
                Scale = 1.1f;
                do {
                    Scale -= 0.1f;
                    DestXSize = (long)((float)MAIN_WindowWidth * Scale);
                    DestYSize = (long)((float)MAIN_WindowHeight * Scale);

                    hrBackgroundImage = (udword *)malloc(DestXSize * DestYSize * 4);
                } while((hrBackgroundImage == NULL) && (Scale > 0.4f));
            }
        }

        //if the memory was not succesfully allocated
        if (hrBackgroundImage == NULL)
        {
            memFree(pTempImage);
            return;
        }

        // Stretch the image to fit the current display size
        IncX = (float)XSize / (float)DestXSize;
        IncY = (float)YSize / (float)DestYSize;

        pDest = (unsigned long*)hrBackgroundImage;
        if((IncX < 1.0f) && (IncY < 1.0f) && (pDest != NULL))
        {
            SrcY = 0.0f;
            for(DestY = 0; DestY < DestYSize; DestY++)
            {
                SrcX = 0.0f;
                for(DestX = 0; DestX < DestXSize; DestX++)
                {
#ifdef ENDIAN_BIG
                    pDest[DestX] = LittleLong( hrGetInterpPixel(pTempImage, XSize, SrcX, SrcY) );
#else
                    pDest[DestX] = hrGetInterpPixel(pTempImage, XSize, SrcX, SrcY);
#endif
                    SrcX += IncX;
                }
                pDest += DestXSize;
                SrcY += IncY;
            }
        }
        else if (pDest != NULL)
        {
            SrcY = 0.0f;
            for(DestY = 0; DestY < DestYSize; DestY++)
            {
                SrcX = 0.0f;
                for(DestX = 0; DestX < DestXSize; DestX++)
                {
                    pRGB = &pTempImage[ (((unsigned long)SrcY * XSize) + (unsigned long)SrcX) * 3 ];

#ifdef ENDIAN_BIG
                    pDest[DestX] = LittleLong( 0xff000000 + ((unsigned long)pRGB[0]) + ((unsigned long)pRGB[1] << 8) + ((unsigned long)pRGB[2] << 16) );
#else
                    pDest[DestX] = 0xff000000 + ((unsigned long)pRGB[0]) + ((unsigned long)pRGB[1] << 8) + ((unsigned long)pRGB[2] << 16);
#endif

                    SrcX += IncX;
                }
                SrcY += IncY;
                pDest += DestXSize;
            }
        }

        hrBackXSize = DestXSize;
        hrBackYSize = DestYSize;
        memFree(pTempImage);
    }
}

#define COORD(S,T,X,Y) \
    glTexCoord2f(S, T); \
    glVertex2f(primScreenToGLX(X), primScreenToGLY(Y));
void hrRectSolidTextured2(rectangle *rect)
{
    glColor3ub(255, 255, 255);

    rndTextureEnvironment(RTE_Replace);
    rndTextureEnable(TRUE);

    glBegin(GL_QUADS);
    COORD(0.0f, 0.0f, rect->x0, rect->y0);
    COORD(0.0f, 1.0f, rect->x0, rect->y1 - 1);
    COORD(1.0f, 1.0f, rect->x1, rect->y1 - 1);
    COORD(1.0f, 0.0f, rect->x1, rect->y0);
    glEnd();

    rndTextureEnable(FALSE);
    rndTextureEnvironment(RTE_Modulate);
}
#undef COORD

//don't mind if this is inefficient as it only
//gets called once per image anyway
void hrDrawFile(char* filename, sdword x, sdword y)
{
    udword handle;
    rectangle rect;
    lifheader* lif = trLIFFileLoad(filename, Pyrophoric);

    rndTextureEnable(TRUE);
    rndAdditiveBlends(FALSE);
    glEnable(GL_BLEND);

    glGenTextures(1, &handle);
    trClearCurrent();
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lif->width, lif->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, lif->data);
    x = feResRepositionX(x);
    y = feResRepositionY(y);
    x -= lif->width >> 1;
    y -= lif->height >> 1;
    rect.x0 = x;
    rect.y0 = y;
    rect.x1 = x + lif->width;
    rect.y1 = y + lif->height;
    hrRectSolidTextured2(&rect);

    glDeleteTextures(1, &handle);
    memFree(lif);

    glDisable(GL_BLEND);
}

void hrDrawBackground(void)
{
    real32 x, y;

    rndClearToBlack();

    // Draw the cached background bitmap using glDrawPixels
    if (hrBackgroundImage)
    {
        x = -((real32)hrBackXSize / (real32)MAIN_WindowWidth);
        y = -((real32)hrBackYSize / (real32)MAIN_WindowHeight);

        rndTextureEnable(FALSE);
        rndLightingEnable(FALSE);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glRasterPos2f(x, y);
        if (hrDrawPixelsSupported())
        {
            glDrawPixels(hrBackXSize, hrBackYSize, GL_RGBA, GL_UNSIGNED_BYTE, hrBackgroundImage);
        }
        else
        {
            glcMatrixSetup(TRUE);
            if (singlePlayerGame)
            {
                glcDisplayRGBABackgroundWithoutScaling((ubyte*)hrBackgroundImage);
            }
            else
            {
                glcDisplayRGBABackgroundScaled((ubyte*)hrBackgroundImage);
            }
            glcMatrixSetup(FALSE);
        }

        if (singlePlayerGame && (hrBackgroundInitFrame & 1))
        {
#ifdef _WIN32
            hrDrawFile("feman\\loadscreen\\ring.lif", 115, 342);
            hrDrawFile("feman\\loadscreen\\arrows.lif", 195, 134);
#else
            hrDrawFile("feman/loadscreen/ring.lif", 115, 342);
            hrDrawFile("feman/loadscreen/arrows.lif", 195, 134);
#endif
        }
    }
}

void hrShutdownBackground(void)
{
    // free the memory required by the cached background bitmap

    if (hrBackgroundImage != NULL)
    {
        free(hrBackgroundImage);
        hrBackgroundImage = NULL;
    }
    hrBackgroundInitFrame = 0;
    hrBackgroundReinit = FALSE;
}

void hrAbortLoadingYes(char *name, featom *atom)
{
    hrAbortLoadingGame = TRUE;

    if (multiPlayerGame)
        SendDroppingOutOfLoad(sigsPlayerIndex);

    if (hrAbortLoadConfirm)
    {
        feScreenDelete(hrAbortLoadConfirm);
        hrAbortLoadConfirm = NULL;
    }
}

void hrAbortLoadingNo(char *name, featom *atom)
{
    if (hrAbortLoadConfirm)
    {
        feScreenDelete(hrAbortLoadConfirm);
        hrAbortLoadConfirm = NULL;
    }
}

void hrChatTextEntry(char *name, featom *atom)
{
    char *string;
    ChatPacket temp;
    sdword     width;
    fonthandle fhsave;
    char  testwidth[MAX_CHATSTRING_LENGTH+40];

    if (FEFIRSTCALL(atom))
    {
        // initialize button here
        ChatTextEntryBox = (textentryhandle)atom->pData;
        uicTextEntryInit(ChatTextEntryBox,UICTE_NoLossOfFocus|UICTE_ChatTextEntry);
        uicTextBufferResize(ChatTextEntryBox,MAX_CHATSTRING_LENGTH-2);
        return;
    }

    switch (uicTextEntryMessage(atom))
    {
        case CM_AcceptText :
            string = ((textentryhandle)atom->pData)->textBuffer;
            sendChatMessage(ALL_PLAYER_MASK^PLAYER_MASK(sigsPlayerIndex),string,(uword)sigsPlayerIndex);
            dbgMessagef("text entry: %s\n",string);
            strcpy(temp.message,string);
            temp.packetheader.frame = (uword)sigsPlayerIndex;
            hrProcessPacket((struct ChatPacket *)&temp);
            uicTextEntrySet(ChatTextEntryBox,"",0);
        break;
        case CM_KeyPressed :
            fhsave = fontMakeCurrent(((textentryhandle)atom->pData)->currentFont); //select the appropriate font
            sprintf(testwidth, "%s >  %s", playerNames[sigsPlayerIndex], ((textentryhandle)atom->pData)->textBuffer);
            width = fontWidth(testwidth);
            fontMakeCurrent(fhsave);
            if (width > (atom->width-30))
            {
                uicBackspaceCharacter((textentryhandle)atom->pData);
            }
        break;
    }
}

#if 0
void hrAbortNetworkGame(char *name, featom *atom)
{
    hrAbortLoadingGame = TRUE;
    dbgAssert(multiPlayerGame);
    SendDroppingOutOfLoad(sigsPlayerIndex);
}

void hrAbortNonNetworkGame(char *name, featom *atom)
{
    hrAbortLoadingGame = TRUE;
}
#endif

HorseRaceBars horseBarInfo;
extern bool8 etgHasBeenStarted;

void horseGetNumBars(HorseRaceBars *horsebars)
{
    sdword i;
    bool enablebar[MAX_POSSIBLE_NUM_BARS];
    real32 totalperc;

    dbgAssert(horseTotalNumBars.numBars > 0);
    for (i=0;i<horseTotalNumBars.numBars;i++)
    {
        switch (i)
        {
            case DOWNLOADMAP_BAR:
                enablebar[i] = autodownloadmapRequired();
                break;

            case UNIVERSE_BAR:
                enablebar[i] = TRUE;
                break;

            case ETG_BAR:
                enablebar[i] = etgHasBeenStarted ? FALSE : TRUE;
                break;

            case TEXTURE1_BAR:
            case TEXTURE2_BAR:
#if TR_NIL_TEXTURE
                enablebar[i] = GLOBAL_NO_TEXTURES ? FALSE : TRUE;
#else
                enablebar[i] = TRUE;
#endif
                break;

            default:
                dbgFatalf(DBG_Loc,"You must specify whether or not to load bar %i here",i);
                break;
        }
    }

    horsebars->numBars = 0;

    // now, based on enablebar[i], selectively add bars that will be loaded:

    for (i=0;i<horseTotalNumBars.numBars;i++)
    {
        if (enablebar[i])
        {
            horsebars->perc[horsebars->numBars++] = horseTotalNumBars.perc[i];
        }
    }

    dbgAssert(horsebars->numBars > 0);

    // now renormalize percentages to 1.0
    for (totalperc = 0.0f,i=0;i<horsebars->numBars;i++)
    {
        totalperc += horsebars->perc[i];
    }

    dbgAssert(totalperc > 0.0f);
    if (totalperc != 1.0f)
    {
        for (i=0;i<horsebars->numBars;i++)
        {
            horsebars->perc[i] /= totalperc;
        }
    }
}

void horseRaceInit()
{
    udword i;
    horseGetNumBars(&horseBarInfo);

    if (glcActive())
    {
        (void)glcActivate(FALSE);
    }

    //initialize current bar to the 0th bar
    localbar=0;

    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        horseracestatus.barnum[i] = 0;
        horseracestatus.percent[i] = 0;
        horseracestatus.hrstatusstr[i][0] = 0;
        if (autodownloadmapRequired())
            TTimerStart(&hrPlayerDropoutTimers[i],HorseRacePlayerDropoutTime*2.0f);     // give double time for autodownloads
        else
            TTimerStart(&hrPlayerDropoutTimers[i],HorseRacePlayerDropoutTime);
        PlayersAlreadyDrawnDropped[i] = FALSE;
    }

    for (i=0;i<NUM_CHAT_LINES;i++)
    {
        chathistory[i].message[0] = 0;
    }

    listInit(&horseCrapRegion.cutouts);

    JustInit = TRUE;

    if (!hrScreensHandle)
    {
        feCallbackAddMultiple(hrCallBack);
        feDrawCallbackAddMultiple(hrDrawCallback);
        hrScreensHandle = feScreensLoad(HR_FIBFile);
    }

    if (!ShouldHaveMousePtr) mouseCursorHide();

    if (singlePlayerGame)
    {
        hrBaseRegion = feScreenStart(&horseCrapRegion, HR_SingleRaceScreen);
    }
    else
    {
        hrBaseRegion = feScreenStart(&horseCrapRegion, (multiPlayerGame) ? HR_RaceScreen : HR_RaceScreenNotNetwork);
    }

    playernamefont = frFontRegister(HR_PlayerNameFont);

    hrRunning=TRUE;
    if (RGLtype == SWtype && feShouldSaveMouseCursor())
    {
        rglFeature(RGL_SAVEBUFFER_ON);
    }
    hrBackgroundDirty = (RGLtype == SWtype) ? 10000 : 3;
    hrBackgroundReinit = FALSE;
}

void horseRaceShutdown()
{
    sdword i;

    if (!ShouldHaveMousePtr) mouseCursorShow();

    for(i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        TTimerClose(&hrPlayerDropoutTimers[i]);
    }

    hrRunning=FALSE;
    if (hrAbortLoadConfirm)
    {
        feScreenDelete(hrAbortLoadConfirm);
        hrAbortLoadConfirm = NULL;
    }

    feScreenDelete(hrBaseRegion);

    hrShutdownBackground();

    hrBaseRegion = NULL;
    hrProgressRegion = NULL;
    hrChatBoxRegion = NULL;
    ChatTextEntryBox = NULL;

    if (RGL)
    {
        if (RGLtype == SWtype)
        {
            rglFeature(RGL_SAVEBUFFER_OFF);
        }
        rglSuperClear();
    }
}

void horseRaceWaitForNetworkGameStartShutdown()
{
    if (multiPlayerGame)
    {
        while (!multiPlayerGameUnderWay) ;
    }
}

real32 lasttime;

void HorseRaceBeginBar(uword barnum)
{
    //udword i;
    HorsePacket packet;
    lasttime = 0.0f;
//    if(utyTeaserHeader) return;

    if(JustInit)
    {
        JustInit = FALSE;
    }
    else
    {
        localbar++;
    }

    //send packet
    packet.packetheader.type = PACKETTYPE_HORSERACE;
    packet.playerindex = (uword) sigsPlayerIndex;
    packet.packetheader.numberOfCommands = 0;                    //don't need probably
    packet.barnum = (uword) localbar;
    packet.percent = horseRaceGetPacketPercent(0.0f);

    if (multiPlayerGame)
    {
        SendHorseRacePacket((ubyte *)&packet,sizeof(HorsePacket));
    }
    else
    {
        recievedHorsePacketCB((ubyte *)&packet,sizeof(HorsePacket));
    }
}

extern void mousePoll();
extern udword regRegionProcess(regionhandle reg, udword mask);
extern sdword regProcessingRegions;
extern sdword regRenderEventIndex;

void hrUncleanDecorative(void)
{
    if (hrDecRegion != NULL)
    {
        hrDecRegion->drawFunction = ferDrawDecorative;
    }
}

void horseRaceRender()
{
    SDL_Event e;

#if defined (_MSC_VER)
    _asm xor eax,eax
    _asm mov regRenderEventIndex, eax
#elif defined (__GNUC__) && defined (__i386__)
    __asm__ __volatile__ (
        "xorl %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (regRenderEventIndex) );
#else
    regRenderEventIndex = 0;
#endif

    //special-case code for double-clicks

/*    if (keyIsHit(LMOUSE_DOUBLE))
    {
        keyPressUp(LMOUSE_DOUBLE);
        utyDoubleClick();
    }
    if (demDemoRecording)
    {
        memcpy((ubyte *)&keyScanCode[0], (ubyte *)&keySaveScan[0], sizeof(keyScanCode));//freeze a snapshot of the key state
        demStateSave();
    }
    else if (demDemoPlaying)
    {
        demStateLoad();
    }*/

    if (hrBackgroundReinit)
    {
        if (hrBackgroundImage != NULL)
        {
            free(hrBackgroundImage);
            hrBackgroundImage = NULL;
        }
        hrBackgroundReinit = FALSE;
        hrBackgroundDirty = (RGLtype == SWtype) ? 10000 : 3;
        hrBackgroundInitFrame = 0;
    }

    // Make sure the Homeworld text gets drawn on the correct frames
    if (hrBackgroundDirty || RGLtype == SWtype)
    {
        regRecursiveSetDirty(&horseCrapRegion);
        hrDecRegion = NULL;
    }
    else
    {
        regionhandle reg;

        // "Clean" the region with the homeworld logo
        // in it so it doesn't re-draw all the time

        reg = horseCrapRegion.child;
        if (reg)
        {
            reg = reg->child;
        }

        while (reg && reg->drawFunction != &ferDrawDecorative)
        {
            reg = reg->next;
        }

        if (reg)
        {
            void regNULLRenderFunction(regionhandle region);
            hrDecRegion = reg;
            reg->drawFunction = regNULLRenderFunction;
        }
    }

    if (TitanActive)
        titanPumpEngine();

    SDL_Delay(0);

    if (!SDL_PollEvent(0))
    {
        regProcessingRegions = TRUE;
        regRegionProcess(horseCrapRegion.child, 0xffffffff);
        regProcessingRegions = FALSE;
        if (ChatTextEntryBox!=NULL)
        {
            bitSet(ChatTextEntryBox->reg.status,RSF_KeyCapture);
            keyBufferClear();
        }
    }
    else
    {
        regRegionProcess(horseCrapRegion.child, 0xffffffff);
        while (SDL_PollEvent(0))
        {
            if (SDL_WaitEvent(&e))
            {
                HandleEvent(&e);

                if (multiPlayerGame)
                {
                    if (keyIsStuck(ESCKEY))
                    {
                        keyClearSticky(ESCKEY);                      //clear the sticky bit
                        if (!hrAbortLoadConfirm)
                        {
                            if (!hrAbortLoadingGame)    // if not already aborting
                            {
                                hrAbortLoadConfirm = feScreenStart(&horseCrapRegion, "AbortLoadConfirm");
                            }
                        }
                        else
                        {
                            feScreenDelete(hrAbortLoadConfirm);
                            hrAbortLoadConfirm = NULL;
                        }
                    }
                }

                regProcessingRegions = TRUE;
                if (hrAbortLoadConfirm!=NULL)
                {
                    ;
                }
                else if (ChatTextEntryBox!=NULL)
                {
                    regRegionProcess(&ChatTextEntryBox->reg, 0xffffffff);
                }
                regProcessingRegions = FALSE;
            }
            if (ChatTextEntryBox!=NULL)
            {
                bitSet(ChatTextEntryBox->reg.status,RSF_KeyCapture);
                keyBufferClear();
            }

            if (TitanActive)
                titanPumpEngine();

            SDL_Delay(0);
        }
    }

    // All of the hacked stuff from the render task

    //glColor3ub(colRed(RND_StarColor), colGreen(RND_StarColor), colBlue(RND_StarColor));

    if (ShouldHaveMousePtr)
    {
        if (feShouldSaveMouseCursor())
        {
            if (RGLtype == SWtype) rglFeature(RGL_SAVEBUFFER_ON);
        }
        else
        {
            if (RGLtype == SWtype) rglFeature(RGL_SAVEBUFFER_OFF);
            rndClearToBlack();
            glClear(GL_DEPTH_BUFFER_BIT);
        }
    }

//    primErrorMessagePrint();
    //default rendering scheme is primitives on any
    //functions which want it off should set it back on when done


    // I know this looks weird, but it's correct
    if(hrBackgroundInitFrame == 1)
        hrInitBackground();


    // When there's no background loaded yet, it fills the screen with black
    if (RGLtype == SWtype)
    {
        hrDrawBackground();
    }
    else
    {
        if (hrBackgroundDirty)
        {
            hrDrawBackground();
        }
        else
        {
            if (hrBackgroundImage != NULL)
            {
                free(hrBackgroundImage);
                hrBackgroundImage = NULL;
            }
        }
    }

    regFunctionsDraw();                                //render all regions
    primErrorMessagePrint();

    hrUncleanDecorative();

    // We want the init code to be called on the 2nd pass.  That way, the screen erases,
    // Then we incur the delay of loading the background.
    // For two frames -after that- we'll draw the background,
    // then just draw the progress bars.
    if(hrBackgroundDirty && hrBackgroundInitFrame)
    {
        if (hrBackgroundDirty > 0) hrBackgroundDirty--;
    }

    hrBackgroundInitFrame++;

    if (ShouldHaveMousePtr)
    {
        // set the cursor type, reset the variables then draw the mouse cursor
        mouseSelectCursorSetting();
        mouseSetCursorSetting();
        if (feShouldSaveMouseCursor())
        {
            mouseStoreCursorUnder();
        }
        mousePoll();
        mouseDraw();                                        //draw mouse atop everything

        if (demDemoPlaying)
        {
            rndShamelessPlug();
        }
    }

    rndFlush();
    if (ShouldHaveMousePtr)
    {
        if (feShouldSaveMouseCursor())
        {
            mouseRestoreCursorUnder();
        }
    }
    primErrorMessagePrint();
}

//function that returns the doctored percentage for conversion of
//multiple bars to a single bar.

real32 horseRaceGetPacketPercent(real32 barPercent)
{
    real32 percent=0.0f;
    sdword i;
    for(i=0;i<localbar;i++)
    {
        //add previous bar percentiles...
        percent += horseBarInfo.perc[i];
    }
    //add on current bars contribution scaled by appropriate ammount
    percent += barPercent*horseBarInfo.perc[localbar];
    return percent;
}

bool HorseRaceNext(real32 percent)
{
    Uint32 lp;
    HorsePacket packet;
    real32 temptime;
    udword i;
    sdword dontrenderhack = FALSE;
    bool sendhrpackethack = FALSE;
//    static real32 lastper = 0.0f;
    static sdword modulusCounter = 0;

    if (percent > 1.0f) percent = 1.0f;
//    if(utyTeaserHeader)  return TRUE;

    /*glClear(GL_COLOR_BUFFER_BIT);
    regFunctionsDraw();                                 //render all regions
    glFlush();                                          //ensure all draws complete*/

    if (TitanActive)
        titanPumpEngine();

    packet.packetheader.type = PACKETTYPE_HORSERACE;
    packet.playerindex = (uword) sigsPlayerIndex;
    packet.packetheader.numberOfCommands = 0;                    //don't need probably
    packet.barnum = (uword) localbar;
    //packet.current = (uword) localcurrent;
    //packet.max = (uword) localmax;

    packet.percent = horseRaceGetPacketPercent(percent);

    //render first
    if (lasttime == 0.0f)
    {
        horseRaceRender();
        dontrenderhack = TRUE;
    }

    if (multiPlayerGame && (startingGameState == AUTODOWNLOAD_MAP) && (sigsPlayerIndex == 0))
    {
        sendhrpackethack = TRUE;
        SendHorseRacePacket((ubyte *)&packet,sizeof(HorsePacket));      // always send horse race packet when autouploading map
    }

    lp = SDL_GetTicks();
    temptime = (float)lp;
    if(temptime - lasttime < horseRaceRenderTime)
    {
        if(lasttime > temptime)
            lasttime = temptime;
        return FALSE;     //don't return
    }
    lasttime = temptime;

//    if(lastper != percent)
//    {

    if (multiPlayerGame)
    {
        if (!sendhrpackethack)
        {
            if (modulusCounter <= 0)
            {
                modulusCounter = 6;
            }
            modulusCounter--;
            if (modulusCounter == 0)
            {
                SendHorseRacePacket((ubyte *)&packet,sizeof(HorsePacket));
            }
        }
    }
    else
    {
        recievedHorsePacketCB((ubyte *)&packet,sizeof(HorsePacket));
    }

//    }
//    lastper = percent;

    if (!dontrenderhack)
    {
        horseRaceRender();
    }

    if(multiPlayerGame)
    {
        for(i=0;i<sigsNumPlayers;i++)
        {
            if (i != sigsPlayerIndex)       // don't check myself
            {
                if (!playerHasDroppedOutOrQuit(i))
                {
                    if (TTimerUpdate(&hrPlayerDropoutTimers[i]))
                    {
                        PlayerDroppedOut(i,TRUE);
                    }
                }
            }
        }

        if(localbar == (horseBarInfo.numBars - 1))
        {
            for(i=0;i<sigsNumPlayers;i++)
            {
                if (!playerHasDroppedOutOrQuit(i))
                {
                    if(horseracestatus.percent[i] < 0.999f)
                    {
                        return FALSE;
                    }
                }
            }
        }
        return TRUE;
    }

    return TRUE;
}

void hrProcessPacket(struct ChatPacket *packet)
{
    sdword  i;
    bool    done=FALSE;

    for (i=0;i<NUM_CHAT_LINES;i++)
    {
        if (chathistory[i].message[0]==0)
        {
            strcpy(chathistory[i].message,(*((ChatPacket *)packet)).message);
            chathistory[i].packetheader.frame = (*((ChatPacket *)packet)).packetheader.frame;
            done = TRUE;
            break;
        }
    }

    if (!done)
    {
        for (i=0;i<NUM_CHAT_LINES-1;i++)
        {
            chathistory[i] = chathistory[i+1];
            strcpy(chathistory[i].message,chathistory[i+1].message);
        }
        strcpy(chathistory[i].message,(*((ChatPacket *)packet)).message);
        chathistory[i].packetheader.frame = (*((ChatPacket *)packet)).packetheader.frame;
    }

    hrDirtyChatBox();
}

void recievedHorsePacketCB(ubyte *packet,udword sizeofpacket)
{
    //captain should have recieved a packet here...
    HorsePacket *hp = (HorsePacket *)packet;
    udword i;

    i = hp->playerindex;
    dbgAssert(i < MAX_MULTIPLAYER_PLAYERS);

    if (multiPlayerGame)
    {
        titanDebug("hr: recv hrpacket from %d status %d",i,playersReadyToGo[i]);
        if (!playerHasDroppedOutOrQuit(i))
        {
            TTimerReset(&hrPlayerDropoutTimers[i]);
        }
    }

    hrDirtyProgressBar();

    horseracestatus.barnum[i] = hp->barnum;
    horseracestatus.percent[i] = hp->percent;
}

