/*=============================================================================
    Name    : ddraw.cpp
    Purpose : non-DirectDraw methods for setting display modes

    Created 7/23/1999 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern "C" unsigned int fullScreen;
extern "C" unsigned int mainDirectDraw;

static bool hwCreated = false;
//static int  hwWidth, hwHeight;
extern "C" int  hwWidth, hwHeight;
//static int  hwDepth = 0;
extern "C" int  hwDepth = 0;


extern "C" unsigned int ddSetRes(int, int, int);
extern "C" unsigned int ddCreateWindow(int, int, int, int);
extern "C" unsigned int ddDeleteWindow(void);
extern "C" unsigned int ddGetDepth(void);
extern "C" unsigned int ddActivate(int activate);


/*-----------------------------------------------------------------------------
    Name        : hwGetDepth
    Description : returns screen bitdepth
    Inputs      :
    Outputs     :
    Return      : bitdepth
----------------------------------------------------------------------------*/
extern "C" unsigned int hwGetDepth(void)
{
    return hwDepth;
}

/*-----------------------------------------------------------------------------
    Name        : hwSetRes
    Description : attempt to set specified display mode, or restore previous
    Inputs      : width, height, depth - display mode characteristics
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int hwSetRes(int width, int height, int depth)
{
    DEVMODE devMode;
    int modeExist, modeSwitch, closeMode = 0;
    int i;

    if (width == -1)
    {
        width  = hwWidth;
        height = hwHeight;
        depth  = hwDepth;
    }

    if (width == 0)
    {
        return (ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_SUCCESSFUL);
    }
    else
    {
        for (i = 0;; i++)
        {
            modeExist = EnumDisplaySettings(NULL, i, &devMode);

            if (!modeExist)
            {
                break;
            }

            if ((devMode.dmBitsPerPel == depth) &&
                (devMode.dmPelsWidth  == width) &&
                (devMode.dmPelsHeight == height))
            {
                modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
                if (modeSwitch == DISP_CHANGE_SUCCESSFUL)
                {
                    hwWidth  = width;
                    hwHeight = height;
                    hwDepth  = depth;
                    return 1;
                }
                if (!closeMode)
                {
                    closeMode = i;
                }
            }
        }

        EnumDisplaySettings(NULL, closeMode, &devMode);
        devMode.dmBitsPerPel = depth;
        devMode.dmPelsWidth  = width;
        devMode.dmPelsHeight = height;
        devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
        if (modeSwitch == DISP_CHANGE_SUCCESSFUL)
        {
            hwWidth  = width;
            hwHeight = height;
            hwDepth  = depth;
            return 1;
        }

        devMode.dmFields = DM_BITSPERPEL;
        modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
        if (modeSwitch == DISP_CHANGE_SUCCESSFUL)
        {
            devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
            modeSwitch = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
            if (modeSwitch == DISP_CHANGE_SUCCESSFUL)
            {
                hwWidth  = width;
                hwHeight = height;
                hwDepth  = depth;
                return 1;
            }
            ChangeDisplaySettings(NULL, 0);
        }

#if 0
        if (modeSwitch == DISP_CHANGE_RESTART)
        {
            //...
        }
        else if (modeSwitch == DISP_CHANGE_BADMODE)
        {
            //...
        }
        else if (modeSwitch == DISP_CHANGE_FAILED)
        {
            //...
        }
        else
        {
            //...
        }
#endif

        hwDepth = 0;
        return 0;
    }
}

/*-----------------------------------------------------------------------------
    Name        : hwDeleteWindow
    Description : restore previous display mode
    Inputs      :
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int hwDeleteWindow(void)
{
    if (mainDirectDraw)
    {
        return ddDeleteWindow();
    }

    if (hwCreated)
    {
        hwCreated = FALSE;
        return hwSetRes(0, 0, 0);
    }
    else
    {
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
    Name        : hwCreateWindow
    Description : set display mode
    Inputs      : ihwnd - window handle
                  width, height, depth - display mode characteristics
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int hwCreateWindow(int ihwnd, int width, int height, int depth)
{
    unsigned int rval;

    if (mainDirectDraw)
    {
        return ddCreateWindow(ihwnd, width, height, depth);
    }

    if (!fullScreen)
    {
        hwCreated = FALSE;
        hwDepth = 0;
        return TRUE;
    }

    rval = hwSetRes(width, height, depth);
    hwCreated = rval;
    return rval;
}

/*-----------------------------------------------------------------------------
    Name        : hwActivate
    Description : restore / set the display mode on minimize / restore
    Inputs      : activate - 0 minimize, 1 restore
    Outputs     :
    Return      : >0 success
----------------------------------------------------------------------------*/
extern "C" unsigned int opReloading;
extern "C" unsigned int hwActivate(int activate)
{
    if (mainDirectDraw)
    {
        return ddActivate(activate);
    }
    if (opReloading)
    {
        return 1;
    }

    if (activate)
    {
        //restore
        return hwSetRes(hwWidth, hwHeight, hwDepth);
    }
    else
    {
        //minimize
        return hwSetRes(0, 0, 0);
    }
}
