/*=============================================================================
    Name    : d3denum.cpp
    Purpose : Direct3D enumeration functions

    Created 10/2/1998 by
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "d3drv.h"
#include "d3dlist.h"
#include "d3dinit.h"

static HRESULT WINAPI enum_d3d_devices_cb(
    GUID* pGUID, LPSTR strDesc, LPSTR strName, LPD3DDEVICEDESC pHALDesc,
    LPD3DDEVICEDESC pHELDesc, LPVOID pvContext)
{
    char* ddName = (char*)pvContext;

    if (pGUID == NULL || pHALDesc == NULL || pHELDesc == NULL)
    {
        return D3DENUMRET_CANCEL;
    }

    BOOL isHardware = (pHALDesc->dwFlags != 0);

    if (!isHardware)
    {
        return D3DENUMRET_OK;
    }

    devlist_t dt;
    if (pGUID == NULL)
    {
        ZeroMemory(&dt.guid, sizeof(GUID));
        dt.pGuid = NULL;
    }
    else
    {
        MEMCPY(&dt.guid, pGUID, sizeof(GUID));
        dt.pGuid = &dt.guid;
    }
    strncpy(dt.ddName, ddName, 127);
    strncpy(dt.descStr, strDesc, 127);
    strncpy(dt.nameStr, strName, 127);

    if (isHardware)
    {
        dt.desc = *pHALDesc;
    }
    else
    {
        dt.desc = *pHELDesc;
    }

    devList.push_back(dt);

    return D3DENUMRET_OK;
}

void enum_d3d_devices(d3d_context* d3d, char* name)
{
    d3d->d3dObject->EnumDevices(enum_d3d_devices_cb, name);
}

static BOOL WINAPI enum_dd_devices_cb(
    GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
    ddlist_t dd;

    dd.ddraw = NULL;

    if (FAILED(DirectDrawCreate(lpGUID, &dd.ddraw, NULL)))
    {
        return D3DENUMRET_OK;
    }

    strncpy(dd.descStr, lpDriverDescription, 127);
    strncpy(dd.nameStr, lpDriverName, 127);

    ddList.push_back(dd);

    return D3DENUMRET_OK;
}

static HRESULT WINAPI enum_display_modes_cb(LPDDSURFACEDESC2 ddsd, LPVOID lpContext)
{
#if LOG_DISPLAY_MODES
    FILE* out = (FILE*)lpContext;
#endif

    if ((ddsd->dwFlags & DDSD_WIDTH) &&
        (ddsd->dwFlags & DDSD_HEIGHT) &&
        (ddsd->dwFlags & DDSD_PIXELFORMAT))
    {
        if (ddsd->ddpfPixelFormat.dwRGBBitCount <= 8)
        {
            goto SKIP_MODE;
        }
        if (ddsd->dwWidth < 640 || ddsd->dwHeight < 480)
        {
            goto SKIP_MODE;
        }
#if LOG_DISPLAY_MODES
        fprintf(out, " %dx%d", (int)ddsd->dwWidth, (int)ddsd->dwHeight);
        fprintf(out, " %d bpp", (int)ddsd->ddpfPixelFormat.dwRGBBitCount);
        fprintf(out, "\n");
#endif
    }

 SKIP_MODE:
    return D3DENUMRET_OK;
}

void enum_display_modes(d3d_context* d3d, char* name)
{
#if LOG_DISPLAY_MODES
    FILE* out = fopen("dd.dat", "wt");
    if (out == NULL)
    {
        return;
    }

    fprintf(out, "DirectDraw device %s\n", name);

    d3d->ddraw4->EnumDisplayModes(0, NULL, (LPVOID)out, enum_display_modes_cb);

    fclose(out);
#else
    d3d->ddraw4->EnumDisplayModes(0, NULL, NULL, enum_display_modes_cb);
#endif
}

void enum_dd_devices(d3d_context* d3d)
{
    HRESULT hr;
    char targetDev[128];

    ddList.erase(ddList.begin(), ddList.end());

    hr = DirectDrawEnumerate(enum_dd_devices_cb, NULL);
    if (FAILED(hr))
    {
        errLog("enum_dd_devices(DirectDrawCreate)", hr);
    }

    devList.erase(devList.begin(), devList.end());

    for (listdd::iterator i = ddList.begin(); i != ddList.end(); ++i)
    {
        d3d->ddraw1 = (*i).ddraw;

        hr = d3d->ddraw1->QueryInterface(IID_IDirectDraw4, (void**)&d3d->ddraw4);
        if (FAILED(hr))
        {
            errLog("enum_dd_devices(QueryInterface[ddraw4])", hr);
            d3d->ddraw1->Release();
            d3d->ddraw1 = NULL;
            continue;
        }

        hr = d3d->ddraw4->QueryInterface(IID_IDirect3D3, (void**)&d3d->d3dObject);
        if (FAILED(hr))
        {
            errLog("enum_dd_devices(QueryInterface[d3d3])", hr);
            d3d->ddraw4->Release();
            d3d->ddraw4 = NULL;
            continue;
        }

        enum_display_modes(d3d, (*i).nameStr);

        enum_d3d_devices(d3d, (*i).nameStr);

        d3d->d3dObject->Release();
        d3d->d3dObject = NULL;

        d3d->ddraw4->Release();
        d3d->ddraw4 = NULL;

        strncpy(targetDev, rglD3DGetDevice(), 127);
        if (strlen(targetDev) > 0)
        {
            if (strcmp((*i).nameStr, targetDev) == 0)
            {
                lpDirectDraw = d3d->ddraw1;
                d3d->ddraw1 = NULL;
//                rglD3DSetDevice(NULL);
                goto FINISH_NOW;
            }
            else
            {
                d3d->ddraw1->Release();
                d3d->ddraw1 = NULL;
            }
        }
        else
        {
            d3d->ddraw1->Release();
            d3d->ddraw1 = NULL;
        }
    }

  FINISH_NOW:
    devList.erase(devList.begin(), devList.end());
}

void enum_dd_cleanup(d3d_context* d3d)
{
#if 0
    FILE* out = fopen("dd.dev", "wt");
    if (out != NULL)
    {
        for (listdd::iterator i = ddList.begin(); i != ddList.end(); ++i)
        {
            fprintf(out, "\ndesc: %s\n", (*i).descStr);
            fprintf(out, "name: %s\n", (*i).nameStr);
        }

        fclose(out);
    }
#endif

    ddList.erase(ddList.begin(), ddList.end());
}

HRESULT WINAPI enum_depthbuffer_cb(DDPIXELFORMAT* ddpf, VOID* userData)
{
    if (ddpf->dwFlags == DDPF_ZBUFFER)
    {
        if (ddpf->dwRGBBitCount > 16)
        {
            hiDepthList.push_back(*ddpf);
        }
        else
        {
            loDepthList.push_back(*ddpf);
        }
    }

    return D3DENUMRET_OK;
}

