/*=============================================================================
    Name    : screenshot.c
    Purpose : take screenshots via OpenGL

    Created 7/15/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "Debug.h"
#include "screenshot.h"
#include "interfce.h"

static void _numberify(char* dirname)
{
    int   i;
    FILE* outfile;
    char  filename[32], testname[PATH_MAX + 1];

    filename[0] = '\0';

    for (i = 0; i < 200; i++)
    {
        sprintf(filename, "shota%03d.jpg", i);
        strcpy(testname, dirname);
        strcat(testname, filename);

        if ((outfile = fopen(testname, "rt")) == NULL)
        {
            strcat(dirname, filename);
            return;
        }

        fclose(outfile);
    }

    strcat(dirname, "shot200.jpg");
}

void ssSaveScreenshot(ubyte* buf)
{
    char *fname;
    FILE* out;
    unsigned char *pTempLine;
    long Top, Bot, i, Size;

    JPEGDATA jp;

    fname = filePathPrepend("ScreenShots/", FF_UserSettingsPath);
    if (!fileMakeDirectory(fname))
        return;

    _numberify(fname);

#if SS_VERBOSE_LEVEL >= 1
    dbgMessagef("\nSaving %dx%d screenshot to '%s'.", MAIN_WindowWidth, MAIN_WindowHeight, fname);
#endif

    out = fopen(fname, "wb");
    if (out == NULL)
    {
        return;
    }

    Size = MAIN_WindowWidth*3;
    pTempLine = (unsigned char *)malloc(Size);

    for (i = 0; i < (MAIN_WindowHeight / 2); i++)
    {
        Top = i;
        Bot = (MAIN_WindowHeight - 1) - i;

        memcpy(pTempLine, buf + (Size * Top), Size);
        memcpy(buf + (Size * Top), buf + (Size * Bot), Size);
        memcpy(buf + (Size * Bot), pTempLine, Size);
    }

    free(pTempLine);

    // Fill out the JPG lib info structure
    memset(&jp, 0, sizeof(jp));

    jp.ptr = buf;
    jp.width = MAIN_WindowWidth;
    jp.height = MAIN_WindowHeight;
    jp.output_file = out;
    jp.aritcoding = 0;
    jp.quality = 97;

    JpegWrite(&jp);

    fclose(out);
}
